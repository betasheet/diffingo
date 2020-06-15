/*
 * main.cpp
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

#include <boost/program_options.hpp>
#include <pantheios/frontends/stock.h>
#include <pantheios/pantheios.hpp>
#include <exception>
#include <iostream>
#include <map>
#include <string>

#include "generation/compiler.h"  // NOLINT

const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = "diffingo";
namespace po = boost::program_options;

bool process_command_line(int argc, char** argv,
                          diffingo::generation::Options* options);

int main(int argc, char** argv) {
  pantheios::log(pantheios::informational,
                 "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  pantheios::log(pantheios::informational,
                 "~------ Diffingo version 0.1 -------~");
  pantheios::log(pantheios::informational,
                 "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

  diffingo::generation::Options options;
  if (!process_command_line(argc, argv, &options)) {
    return 1;
  }

  diffingo::generation::Compiler compiler(&options);
  return compiler.run();
}

bool process_command_line(int argc, char** argv,
                          diffingo::generation::Options* options) {
  try {
    po::options_description desc("Program Usage", 1024, 512);
    desc.add_options()                    //
        ("help", "produce help message")  //
        ("file,f", po::value<std::string>(&options->file)->required(),
         "the diffingo spec input file")  //
        ("namespace,n",
         po::value<std::string>(&options->name_space)->required(),
         "the namespace to be used for the output file")  //
        ("output,o", po::value<std::string>(&options->output_dir)->required(),
         "the dir to save output files in")  //
        ("ast,a", po::bool_switch(&options->print_ast)->default_value(false),
         "print ast")  //
        ("instantiation_only,i",
         po::bool_switch(&options->instantiation_only)->default_value(false),
         "only translate instantiated types")  //
        ("input_pointers,p",
         po::bool_switch(&options->input_pointers)->default_value(false),
         "use pointers into input buffers for unused fields")  //
        ("store_parsing_only,s",
         po::bool_switch(&options->store_parsing_only)->default_value(true),
         "store parsing-only variable/field values within parsed units")  //
        ;  // NOLINT

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return false;
    }

    po::notify(vm);
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unknown error!" << std::endl;
    return false;
  }

  return true;
}
