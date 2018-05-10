# Coddle

Yet another build system. :)

Your project have only C/C++ source code, the tool is figuring out and installing all dependencies automatically. No config or make files required.

You create source files such as src1.cpp, src2.cpp, header1.hpp, header2.hpp in the directory of your project "prj". When type:

```
$ coddle
```

The build system will automatically figure out how to make the binary out of your source code. No configuration file is required at all.

The goal is to make it work for Windows, Linux and Mac OS X.

## Problems

- Signing PGP

- Compatibility with configure scripts

- Publishing libraries

- Versioning

- Working with existing packaging infrastructure: deb, rpm, package config, autoconfig

Support for different compilers: gcc, clang, mingw, IBM C++, Intel C++, Microsoft Visual C++.

## Deployment

```
$ git clone https://github.com/antonte/coddle.git && coddle/build.sh
$ sudo coddle/deploy.sh
```

Toolchain interface
compiling:

+ list of includes,

list of symbols (provided and needed)

linking
generating dependency

generic flags:
c vs c++
03/11/14/17
dynamic/static
single/multithreaded
compiler specific flags

Create dynamic library out of the config, link and run

Binary depends out of the config dynamic library

Resolver runs the code of the dynamic library

map: target file, resolver

Config is C++ code

Include files of library are symlinked in the special location.

If library dynamic library .so file or .dll file symlink to the library creates in the special location.

Coddle at start up loads map symbol to the object information also symbol to the dynamic library.

External libraries are source code on git

There is a special git repository with mapping header file to the external library - library repository.

The point️ to the library repository is configurable. You can have multiple library repositories.

Also you can specify the version of the library in the config. The version is a tag, branch or git hash.

Solution

Global libraries, solution libraries.

Config may have different flavors: release, debug, 64 bit, 32 bit, Linux, Windows, Mac OS x, ARM, iOS.

For each flavor temporal directory is created during the compilation. As an example: .coddle/debug, or .coddle/debug-x64-linux.

Flavors are set up in configuration, and should be selected from the command line.

Dependency build
================

- directory
- list of source files
- packages

compile object file: source file, packges, includes, cflags

link: object files, packages, libraries, ldflags

