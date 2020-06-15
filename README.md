Welcome! This is the place to get going with Diffingo :)

# Background & Details

See [diffingo.pdf](diffingo.pdf).

# Dependencies
We have tested building with GCC 4.8.
This project brings most of its dependencies with it, except for the following:

* cmake
* FLEX
* BISON
* BOOST libraries (v1.40+): program_options, filesystem, and system

# Building
This project uses cmake as its build system.

To set up building inside the source directory, run `./create_eclipse_project.sh` in the project root.
To set up an external build directory (untested), create a new directory elsewhere and execute `cmake [path to project root]`.

To build the project, run `make`.

During the first build, you will encounter errors at link time.
This is because auto-generated files (for the language parser) were not yet added to the make files by cmake.
To fix it, simply rerun cmake (as above) and `make` again.

# Running
Building generates a Diffingo executable in the src directory of the build directory (or source directory for in-source builds).

To view a list of command options, run `./Diffingo_exec --help`.

For example, to generate the Memcached parser and serializer, run
`./Diffingo_exec -f examples/memcached.dgo -o examples/out_test -n memcached`.

To generate the optimized parser and serializer for its instantiation, run
`./Diffingo_exec -f examples/memcached_inst.dgo -o examples/out_test -i -n memcached_compact`.
Add command line option `-p` to generate parsers and serializers that use pointers into the input buffer.

# License
See [LICENSE](LICENSE) file.
