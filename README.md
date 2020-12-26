# Mineola
OpenGL ES based minimal rendering engine.

## Build on Ubuntu 18.04

We have switched to using [Conan](https://conan.io) to manage dependencies. To build mineola under Linux, please run conan and cmake to generate makefile and build the project:
```bash
$ mkdir build
$ cd build
$ conan install ..
$ cmake ../src
```

## Build for iOS on MacOS

We are also using CMake to generate the XCode project.

First create an iOS conan profile following [imgpp](https://github.com/shenfy/imgpp#build-from-source-for-ios-w-conan).

Then:
```bash
$ mkdir build
$ cd build
$ conan install --profile ios ..
$ cmake ../src -GXcode -DCMAKE_SYSTEM_NAME=iOS
```
Then you can either use cmake/xcode command line tools to build the project or open the generated project `mineola` directly in XCode GUI.

## Build for Android

Please use the included conanfile and CMake script to build the dynamic-link library libmineola.so for your target arch/api_level:
```bash
$ NDK=/path/to/ndk/build/cmake/android.toolchain.cmake conan create .. (user)/(channel) -s os=Android -s os.api_level=(min api) -s arch=(arch)
```

Then request the mineola shared library in your Android project by editing the build.gradle script.
Please see the included gltfview example Android Studio project for details.
(p.s. the project targets x86 emulator by default, feel free to try changing the target arch/api_level)
