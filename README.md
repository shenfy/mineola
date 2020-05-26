# Mineola
OpenGL ES based minimal rendering engine.

## Build on Ubuntu 18.04

We have switched to using [Conan](https://conan.io) to manage dependencies. To build mineola under Linux, please run conan and cmake to generate makefile and build the project:
```bash
$ cd pc
$ mkdir build
$ cd build
$ conan install ..
$ cmake ..
$ make -j8
```

## Build for iOS on MacOS

We have switched to using CMAKE to generate the XCode project.
```bash
$ cd ios
$ mkdir build
$ cd build
$ conan install ..
$ cmake .. -GXcode -DCMAKE_SYSTEM_NAME=iOS
```
Then you can either use cmake/xcode command line tools to build the project or open the generated project "mineola_ios" directly in XCode GUI.