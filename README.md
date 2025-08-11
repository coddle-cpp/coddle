# Coddle

[![Build Status](https://travis-ci.com/coddle-cpp/coddle.svg?branch=master)](https://travis-ci.com/coddle-cpp/coddle)

Yet another build and package system. :)

If your project contains only C/C++ source code, Coddle will discover and
install dependencies automatically. No config file or Makefile required.

Create source files (for example, `src1.cpp`, `src2.cpp`, `header1.hpp`, `header2.hpp`) in
your project directory (for example, `prj/`). Then run:

```
$ coddle
```

The build system automatically figures out how to build a binary from your
source code. No configuration file is required.

Targets: Windows, Linux, macOS, and iOS (Android maybe).

Coddle currently supports Clang only.

## Dependencies

- A supported computer/OS
- git
- clang
- Internet access (to fetch dependencies)

### Linux
- See above

### macOS
- Homebrew if you use pkg-config

### Windows
- MSYS2

## Deployment

### Linux/macOS
```
$ git clone https://github.com/coddle-cpp/coddle.git && cd coddle && ./build.sh && sudo ./deploy.sh; cd ..
```

### Windows
```
$ git clone https://github.com/coddle-cpp/coddle.git && cd coddle && ./win_build_deploy.sh; cd ..
```

## Configuration

Sometimes you will want a configuration file.

The config file is `coddle.toml`.

### target

The name of your binary or library.

Default: the name of the current directory.

Example:

```
target="my_new_project"
```

### remoteRepository

The Git URL of the package repository.

Default: `https://github.com/coddle-cpp/coddle-repository.git`

Example:
```
remoteRepository="https://github.com/coddle-cpp/coddle-repository.git"
```

### remoteVersion
Git branch or tag for the package repository.

Default: `master`; `win` for Windows and `macosx` for macOS.

Example:

```
remoteVersion="master"
```

### localRepository
Path to a local repository. Values from the local repository override values from the remote repository.

Default: empty string.

Example:
```
localRepository="../coddle-repository"
```

### srcDir
Path to the directory containing source files.

Default: `.` (current directory).

Example:
```
srcDir="src"
```

### targetDir
Path where Coddle should place the binary.
Default: `.` (current directory).

Example:
```
targetDir="bin"
```
### artifactsDir
Path where Coddle should put build artifacts.

Default: `.coddle`

Example:
```
artifactsDir="build"
```

### debug
Controls optimization level and debug symbols. If `debug` is set, optimization is
disabled and symbols are included in the binary.

Default: `false`

Example:
```
debug=true
```

You can also set debug from the command line:
```
coddle debug
```

### multithreaded

Specifies whether your project uses threads.

Default: `false`

This is not detected automatically.

Example:
```
multithreaded=true
```

### winmain

Specifies whether your project is a Windows GUI application.

Default: `false`

Example:
```
winmain=true
```

### shared

If `true`, produces a shared library instead of a static one.

Default: `false`
Example:
```
shared=true
```

### cc

Override C compiler.

Default: `clang`
Example:
```
cc="clang"
```

### cxx

Override C++ compiler.

Default: `clang++`
Example:
```
cxx="clang++"
```

### marchNative

Specifies whether Coddle should add `-march=native` to the
compiler flags.

Default: `true`
Example:
```
marchNative=false
```

## Repository format

Your repository should have a `libraries.toml` file. The file is a list of libraries.

Example entry:

```
[[library]]
type="git"
name="cpptoml"
path="https://github.com/skystrife/cpptoml.git"
includes=["cpptoml.h"]
incdir="include"
incdirs=["include1", "include2"]
```

### type
Library type. Possible values: `git`, `pkgconfig`, `lib`.

Example:
```
type="lib"
```

### name
Name of the library.
For `pkgconfig`, the name must match the pkg-config package name.

Example:
```
name="cpptoml"
```

### path
Path to the library. It might be a Git URL or a filesystem path.

Example:
```
path="https://github.com/skystrife/cpptoml.git"
```

### version
Only for `git` type: branch or tag.

Default: `master`

Example:
```
version="1.0.1"
```

### postClone
One-line bash script to run after cloning the Git repository.

### includes
Coddle detects dependencies automatically based on files included in
your sources. The `includes` array maps header names to dependent libraries.

Example:
```
includes=["cpptoml.h"]
```

### incdir or incdirs
The directory (or directories) where the library's header files are located.

Example:
```
incdir="include"

```

### libdir
The directory where the library file is located (`-L` link option).
Example:
```
libdir="libs"
```

## Q\&A

**Q:** I just created a new source file `gameplay.cpp`. Do I need to edit some `.toml`, `.json`, or `.ini` file?  
**A:** Nope. Just run:

```
coddle
```

Coddle will sniff it out and build it. Like a truffle pig, but for code. 🐷

---

**Q:** How do I tell it to build? Do I need `--build`, `--compile`, `--make-it-go-fast` or something?  
**A:** No. Just run:

```
coddle
```

That’s it. The command you can’t forget.

---

**Q:** I want to add a library like SDL2. Do I need to configure a bunch of include paths?  
**A:** Nope. Just put this in your code:

```cpp
#include <SDL.h>
```

Then run:

```
coddle
```

Coddle will fetch it for you. (It’s clingy like that.)

---

**Q:** I don’t trust this “automatic” thing. I want a clean build from scratch.  
**A:** Usually, just run:

```
coddle
```

Coddle will rebuild what’s needed. Only if you think the build artifacts are corrupted should you drop the hammer:

```
rm -rf .coddle
```

That’s the nuclear option. ☢️

---

**Q:** How do I make a debug build?  
**A:** Just run:

```
coddle debug
```

Debugging has never been so… undramatic.

---

**Q:** How do I run the built app with Coddle? I tried `coddle --run` or `coddle run`, but that doesn’t work.  
**A:** Because Coddle builds, it doesn’t babysit. Once built, just run:

```
./your-app
```

Coddle trusts you to press Enter on your own.

---

**Q:** How do I make the build use multiple CPU cores? Do I need the `-j` flag?  
**A:** You already know the answer:

```
coddle
```

It’ll use all your cores without asking. 🖥️💨
