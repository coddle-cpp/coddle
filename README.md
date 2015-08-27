# Coddle

Yet another build system. :)

Idea of the build system is no configuration files at all for your project.

You make source files src1.cpp, src2.cpp, header1.hpp, header2.hpp in the directory of your project "prj". When type:

```
$ coddle
```

And the build system automatically figuring out how to make the binary out of your source code. No configuration file required at all.

The goal is make it work for Windows, Linux and Mac OS X.

Signing PGP.

Compatibility with configure scripts.

Deployment.

```
git clone https://github.com/antonte/coddle.git && coddle/build.sh
sudo coddle/deploy.sh
```
