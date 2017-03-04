# Coddle

Yet another build system. :)

Your project have only C/C++ source code, the tool figuring out and installing all dependencies automatically. No config/make files required.

You make source files src1.cpp, src2.cpp, header1.hpp, header2.hpp in the directory of your project "prj". When type:

```
$ coddle
```

And the build system automatically figuring out how to make the binary out of your source code. No configuration file required at all.

The goal is make it work for Windows, Linux and Mac OS X.

## Problems

Signing PGP

Compatibility with configure scripts

Publishing libraries

Versioning

Working with existing packaging infrastructure deb, rpm.

Support for different compilers: gcc, clang, mingw, IBM C++, Intel C++, Microsoft Visual C++.

## Deployment

```
$ git clone https://github.com/antonte/coddle.git && coddle/build.sh
$ sudo coddle/deploy.sh
```
