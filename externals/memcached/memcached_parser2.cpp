/*
 * memcached_parser.cpp
 *
 * Distributed under the MIT License (MIT).
 *
 * Copyright (c) 2015 Eric Seckler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "memcached_parser.h"  // NOLINT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>

static int add_iov(conn *c, const void *buf, int len);
static int add_msghdr(conn *c);

/**
 * get a pointer to the start of the request struct for the current command
 */
static void *binary_get_request(conn *c) {
  char *ret = c->rcurr;
  ret -= (sizeof(c->binary_header) + c->binary_header.request.keylen +
          c->binary_header.request.extlen);

  assert(ret >= c->rbuf);
  return ret;
}

/**
 * get a pointer to the key in this request
 */
static char *binary_get_key(conn *c) {
  return c->rcurr - (c->binary_header.request.keylen);
}

static void add_bin_header(conn *c, uint16_t err, uint8_t hdr_len,
                           uint16_t key_len, uint32_t body_len) {
  protocol_binary_response_header *header;

  assert(c);

  c->msgcurr = 0;
  c->msgused = 0;
  c->iovused = 0;
  if (add_msghdr(c) != 0) {
    /* This should never run out of memory because iov and msg lists
     * have minimum sizes big enough to hold an error response.
     */
    fprintf(stderr, "SERVER_ERROR out of memory adding binary header\n");
    abort();
  }

  header = (protocol_binary_response_header *)c->wbuf;

  header->response.magic = (uint8_t)PROTOCOL_BINARY_RES;
  header->response.opcode = c->binary_header.request.opcode;
  header->response.keylen = (uint16_t)htons(key_len);

  header->response.extlen = (uint8_t)hdr_len;
  header->response.datatype = (uint8_t)PROTOCOL_BINARY_RAW_BYTES;
  header->response.status = (uint16_t)htons(err);

  header->response.bodylen = htonl(body_len);
  header->response.opaque = c->opaque;
  header->response.cas = bswap_64(c->cas);

  if (settings.verbose > 1) {
    int ii;
    fprintf(stderr, ">%d Writing bin response:", c->sfd);
    for (ii = 0; ii < sizeof(header->bytes); ++ii) {
      if (ii % 4 == 0) {
        fprintf(stderr, "\n>%d  ", c->sfd);
      }
      fprintf(stderr, " 0x%02x", header->bytes[ii]);
    }
    fprintf(stderr, "\n");
  }

  add_iov(c, c->wbuf, sizeof(header->response));
}

/* Form and send a response to a command over the binary protocol */
static void write_bin_response(conn *c, void *d, int hlen, int keylen,
                               int dlen) {
  if (!c->noreply || c->cmd == PROTOCOL_BINARY_CMD_GET ||
      c->cmd == PROTOCOL_BINARY_CMD_GETK) {
    add_bin_header(c, 0, hlen, keylen, dlen);
    if (dlen > 0) {
      add_iov(c, d, dlen);
    }
    conn_set_state(c, conn_mwrite);
    c->write_and_go = conn_new_cmd;
  } else {
    conn_set_state(c, conn_new_cmd);
  }
}

static void bin_read_key(conn *c, enum bin_substates next_substate, int extra) {
  assert(c);
  c->substate = next_substate;
  c->rlbytes = c->keylen + extra;

  /* Ok... do we have room for the extras and the key in the input buffer? */
  ptrdiff_t offset =
      c->rcurr + sizeof(protocol_binary_request_header) - c->rbuf;
  if (c->rlbytes > c->rsize - offset) {
    size_t nsize = c->rsize;
    size_t size = c->rlbytes + sizeof(protocol_binary_request_header);

    while (size > nsize) {
      nsize *= 2;
    }

    if (nsize != c->rsize) {
      if (settings.verbose > 1) {
        fprintf(stderr, "%d: Need to grow buffer from %lu to %lu\n", c->sfd,
                (unsigned long)c->rsize, (unsigned long)nsize);
      }
      char *newm = realloc(c->rbuf, nsize);
      if (newm == NULL) {
        fprintf(stderr, "%d: Failed to grow buffer..\n",
                c->sfd);
        abort();
      }

      c->rbuf = newm;
      /* rcurr should point to the same offset in the packet */
      c->rcurr = c->rbuf + offset - sizeof(protocol_binary_request_header);
      c->rsize = nsize;
    }
    if (c->rbuf != c->rcurr) {
      memmove(c->rbuf, c->rcurr, c->rbytes);
      c->rcurr = c->rbuf;
      if (settings.verbose > 1) {
        fprintf(stderr, "%d: Repack input buffer\n", c->sfd);
      }
    }
  }

  /* preserve the header in the buffer.. */
  c->ritem = c->rcurr + sizeof(protocol_binary_request_header);
  conn_set_state(c, conn_nread);
}

static void dispatch_bin_command(conn *c) {
  int protocol_error = 0;

  int extlen = c->binary_header.request.extlen;
  int keylen = c->binary_header.request.keylen;
  uint32_t bodylen = c->binary_header.request.bodylen;

  c->noreply = true;

  switch (c->cmd) {
    case PROTOCOL_BINARY_CMD_SET: /* FALLTHROUGH */
    case PROTOCOL_BINARY_CMD_ADD: /* FALLTHROUGH */
    case PROTOCOL_BINARY_CMD_REPLACE:
      if (extlen == 8 && keylen != 0 && bodylen >= (keylen + 8)) {
        bin_read_key(c, bin_reading_set_header, 8);
      } else {
        protocol_error = 1;
      }
      break;
    case PROTOCOL_BINARY_CMD_GETQ:  /* FALLTHROUGH */
    case PROTOCOL_BINARY_CMD_GET:   /* FALLTHROUGH */
    case PROTOCOL_BINARY_CMD_GETKQ: /* FALLTHROUGH */
    case PROTOCOL_BINARY_CMD_GETK:
      if (extlen == 0 && bodylen == keylen && keylen > 0) {
        bin_read_key(c, bin_reading_get_key, 0);
      } else {
        protocol_error = 1;
      }
      break;
    case PROTOCOL_BINARY_CMD_DELETE:
      if (keylen > 0 && extlen == 0 && bodylen == keylen) {
        bin_read_key(c, bin_reading_del_header, extlen);
      } else {
        protocol_error = 1;
      }
      break;
    case PROTOCOL_BINARY_CMD_INCREMENT:
    case PROTOCOL_BINARY_CMD_DECREMENT:
      if (keylen > 0 && extlen == 20 && bodylen == (keylen + extlen)) {
        bin_read_key(c, bin_reading_incr_header, 20);
      } else {
        protocol_error = 1;
      }
      break;
    case PROTOCOL_BINARY_CMD_APPEND:
    case PROTOCOL_BINARY_CMD_PREPEND:
      if (keylen > 0 && extlen == 0) {
        bin_read_key(c, bin_reading_set_header, 0);
      } else {
        protocol_error = 1;
      }
      break;
  }
}

static void complete_nread_binary(conn *c) {
  assert(c != NULL);
  assert(c->cmd >= 0);

  switch (c->substate) {
    case bin_reading_set_header:
      if (c->cmd == PROTOCOL_BINARY_CMD_APPEND ||
          c->cmd == PROTOCOL_BINARY_CMD_PREPEND) {
        // process_bin_append_prepend(c);
      } else {
        process_bin_update(c);
      }
      break;
    case bin_read_set_value:
      complete_update_bin(c);
      break;
    case bin_reading_get_key:
    case bin_reading_touch_key:
      process_bin_get_or_touch(c);
      break;
    case bin_reading_stat:
      process_bin_stat(c);
      break;
    case bin_reading_del_header:
      process_bin_delete(c);
      break;
    case bin_reading_incr_header:
      complete_incr_bin(c);
      break;
    case bin_read_flush_exptime:
      process_bin_flush(c);
      break;
    case bin_reading_sasl_auth:
      process_bin_sasl_auth(c);
      break;
    case bin_reading_sasl_auth_data:
      process_bin_complete_sasl_auth(c);
      break;
    default:
      fprintf(stderr, "Not handling substate %d\n", c->substate);
      assert(0);
  }
}

/*
 * if we have a complete line in the buffer, process it.
 */
static int try_read_command(conn *c) {
  assert(c != NULL);
  assert(c->rcurr <= (c->rbuf + c->rsize));
  assert(c->rbytes > 0);

  /* Do we have the complete packet header? */
  if (c->rbytes < sizeof(c->binary_header)) {
    /* need more data! */
    return 0;
  } else {
#ifdef NEED_ALIGN
    if (((long)(c->rcurr)) % 8 != 0) {
      /* must realign input buffer */
      memmove(c->rbuf, c->rcurr, c->rbytes);
      c->rcurr = c->rbuf;
      if (settings.verbose > 1) {
        fprintf(stderr, "%d: Realign input buffer\n", c->sfd);
      }
    }
#endif
    protocol_binary_request_header *req;
    req = (protocol_binary_request_header *)c->rcurr;

    if (settings.verbose > 1) {
      /* Dump the packet before we convert it to host order */
      int ii;
      fprintf(stderr, "<%d Read binary protocol data:", c->sfd);
      for (ii = 0; ii < sizeof(req->bytes); ++ii) {
        if (ii % 4 == 0) {
          fprintf(stderr, "\n<%d   ", c->sfd);
        }
        fprintf(stderr, " 0x%02x", req->bytes[ii]);
      }
      fprintf(stderr, "\n");
    }

    c->binary_header = *req;
    c->binary_header.request.keylen = ntohs(req->request.keylen);
    c->binary_header.request.bodylen = ntohl(req->request.bodylen);
    c->binary_header.request.cas = bswap_64(req->request.cas);

    if (c->binary_header.request.magic != PROTOCOL_BINARY_REQ) {
      if (settings.verbose) {
        fprintf(stderr, "Invalid magic:  %x\n", c->binary_header.request.magic);
      }
      abort();
    }

    c->msgcurr = 0;
    c->msgused = 0;
    c->iovused = 0;
    if (add_msghdr(c) != 0) {
      fprintf(stderr, "SERVER_ERROR Out of memory allocating headers\n");
      abort();
    }

    c->cmd = c->binary_header.request.opcode;
    c->keylen = c->binary_header.request.keylen;
    c->opaque = c->binary_header.request.opaque;
    /* clear the returned cas value */
    c->cas = 0;

    dispatch_bin_command(c);

    c->rbytes -= sizeof(c->binary_header);
    c->rcurr += sizeof(c->binary_header);
  }

  return 1;
}


/*
 * Adds data to the list of pending data that will be written out to a
 * connection.
 *
 * Returns 0 on success, -1 on out-of-memory.
 */

static int add_iov(conn *c, const void *buf, int len) {
    struct msghdr *m;
    int leftover;
    bool limit_to_mtu;

    assert(c != NULL);

    do {
        m = &c->msglist[c->msgused - 1];

        /*
         * Limit UDP packets, and the first payloads of TCP replies, to
         * UDP_MAX_PAYLOAD_SIZE bytes.
         */
        limit_to_mtu = IS_UDP(c->transport) || (1 == c->msgused);

        /* We may need to start a new msghdr if this one is full. */
        if (m->msg_iovlen == IOV_MAX ||
            (limit_to_mtu && c->msgbytes >= UDP_MAX_PAYLOAD_SIZE)) {
            add_msghdr(c);
            m = &c->msglist[c->msgused - 1];
        }

        if (ensure_iov_space(c) != 0)
            return -1;

        /* If the fragment is too big to fit in the datagram, split it up */
        if (limit_to_mtu && len + c->msgbytes > UDP_MAX_PAYLOAD_SIZE) {
            leftover = len + c->msgbytes - UDP_MAX_PAYLOAD_SIZE;
            len -= leftover;
        } else {
            leftover = 0;
        }

        m = &c->msglist[c->msgused - 1];
        m->msg_iov[m->msg_iovlen].iov_base = (void *)buf;
        m->msg_iov[m->msg_iovlen].iov_len = len;

        c->msgbytes += len;
        c->iovused++;
        m->msg_iovlen++;

        buf = ((char *)buf) + len;
        len = leftover;
    } while (leftover > 0);

    return 0;
}


/*
 * Adds a message header to a connection.
 *
 * Returns 0 on success, -1 on out-of-memory.
 */
static int add_msghdr(conn *c)
{
    struct msghdr *msg;

    assert(c != NULL);

    if (c->msgsize == c->msgused) {
        msg = realloc(c->msglist, c->msgsize * 2 * sizeof(struct msghdr));
        if (! msg) {
            return -1;
        }
        c->msglist = msg;
        c->msgsize *= 2;
    }

    msg = c->msglist + c->msgused;

    /* this wipes msg_iovlen, msg_control, msg_controllen, and
       msg_flags, the last 3 of which aren't defined on solaris: */
    memset(msg, 0, sizeof(struct msghdr));

    msg->msg_iov = &c->iov[c->iovused];

    if (IS_UDP(c->transport) && c->request_addr_size > 0) {
        msg->msg_name = &c->request_addr;
        msg->msg_namelen = c->request_addr_size;
    }

    c->msgbytes = 0;
    c->msgused++;

    return 0;
}
