# Coddle

[![Build Status](https://travis-ci.org/coddle-cpp/coddle.svg?branch=master)](https://travis-ci.org/coddle-cpp/coddle)

Yet another build and package system. :)

Your project have only C/C++ source code, the tool is figuring out and
installing all dependencies automatically. No config or make files required.

You create source files such as src1.cpp, src2.cpp, header1.hpp, header2.hpp in
the directory of your project "prj". Type:

```
$ coddle
```

The build system will automatically figure out how to make the binary out of
your source code. No configuration file is required at all.

The goal is to make it work for Windows, Linux, Mac OS X and iOS (Android maybe).

Desision was made to support only clang.

## Dependencies

- computer or laptop
- git
- clang
- access to Internet (sorry the tool would work purly without Internet)

### Linux
- see above

### MacOSX
- brew if you want to use pkgconfig

### Windows
- for some reasons clang depends out of Visual Studio
- bash, but it comes with git

## Deployment

### Linux/MacOSX
```
$ git clone https://github.com/coddle-cpp/coddle.git && cd coddle && ./build.sh && sudo ./deploy.sh; cd ..
```

### Windows
```
$ git clone https://github.com/coddle-cpp/coddle.git && cd coddle && ./win_build_deploy.sh; cd ..
```

## Configuration

Sometimes you would need a configuration.

The config file is coddle.toml

### target

The name of your binary or library.

Default value is the name of the directory.

Example:

```
target="my_new_project"
```

### remoteRepository

The git link to the package repository.

Default value: ```https://github.com/coddle-cpp/coddle-repository.git```

Example:
```
remoteRepository="https://github.com/coddle-cpp/coddle-repository.git"
```

### remoteVersion
Git branch or tag for the package repository.

Default value: ```master```, ```win``` for Windows and ```macosx``` for MacOSX.

Example:

```
remoteVersion="master"
```

### localRepository
Path to the local repository. Values of the local repository override values from remote repository.

Default value is empty string.

Example:
```
localRepository="../coddle-repository"
```

### srcDir
Path to the directory where located source files.

Default value: ```.``` current directory.

Example:
```
srcDir="src"
```

### targetDir
Path where coddle should put binary.
Default value: ```.``` current directory.

Example:
```
targetDir="bin"
```
### artifactsDir
Path where coddle should put build artifacts.

Default value: ```.coddle```

Example:
```
artifactsDir="build"
```

### debug
Sets the level of optimization and symbols. If debug set optimization is
disabled and symbols are included to the binary.

Default value: ```false```

Example:
```
debug=true
```

You also can set debug from command line:
```
coddle debug
```

### multithreaded

Specifies if your project uses threads.

Default value: ```false```

I was not able to figure out how to detect it automatically. Sorry...

Example:
```
multithreaded=true
```

### winmain

Specifies if your project is Windows application.

Default value: ```false```

Example:
```
winmain=true
```

## Repository format

Your repository should have libraries.toml file. In the file it just a list of libraries.

Example of one entry:

```
[[library]]
type="git"
name="cpptoml"
path="https://github.com/skystrife/cpptoml.git"
includes=["cpptoml.h"]
incdir="include"
```

### type
Library type. Possible values: ```git```, ```pkgconfig```, ```lib```.

Example:
```
type="lib"
```

### name
Name of the library.
In case of pgkconfig name has to match with the pkgconfig name.

Example:
```
name="cpptoml"
```

### path
Path to the library. It might be git URL or path to the directory.

Example:
```
path="https://github.com/skystrife/cpptoml.git"
```

### version
Only for git type, branch or tag.

Default value: ```master```

Example:
```
version="1.0.1"
```

### postClone
One line bash script after cloning the git repository.

### includes
Coddle detects dependencies automatically based of on included files in
the source files. includes array makes the mapping between includes in your
source code and dependent libraries.

Example:
```
includes=["cpptoml.h"]
```

### incdir
Path where library include files are located.

Example:
```
incdir="include"

```

### libdir
Path where library file is located (-L clang link option).

Example:
```
libdir="libs"
```
