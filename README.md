# Mineola
An OpenGL ES based minimal rendering engine.
Works on Linux desktop/server (with GLX/EGL), iOS/MacOS and Android.

## Build under Linux (tested under Ubuntu 20.04 LTS and 22.04 LTS)

We have switched to using vcpkg to manage dependencies.
To build and install mineola under Linux, please install vcpkg first, set the $VCPKG_ROOT environment variable, and run the following commands:
```bash
$ mkdir build
$ cmake -S src -B build --preset={GLX|EGL}\
> -DCMAKE_BUILD_TYPE={Debug|Release|MinSizeRel|RelWithDebInfo}  # default=Release
$ cmake --build build --target=install
```

## Run desktop examples
Run the examples inside the build/apps directory:
```bash
$ cd build/apps
$ ./cloth  # press P to run simulation, Q to exit
$ ./ocean
$ ./gltfview resrc/CesiumMan.glb  # press P to play animation, Q to exit
```

## Build in NVidia Cuda/GL docker

For building inside NVidia GPU enabled containers (eg. nvcr.io/nvidia/cudagl), use or modify the provided Dockerfile.build.
For example, to build a headless EGL based server library, use the following commands:
```bash
$ docker build -f Dockerfile.build -t {channel}/mineola .
$ docker run --gpus=all --it -v .:/mineola {channel}/mineola
# inside docker
root:/$ cd mineola
root:/mineola$ cmake -S src -B build --preset=EGL
root:/mineola$ cmake --build build --target=install -j12
root:/mineola$ cd build
root:/mineola/build$ cpack
```
This will produce a zip archive containing all files need for deployment.

## Use in NVidia Cuda/GL docker

Download from the release section or build your own as demonstrated above to get the mineola-1.3.0-Linux.zip archive.
To use the generated archive for app development, extract the contents into /usr/local:
```bash
# inside docker
root:/$ unzip mineola-1.3.0-Linux.zip
root:/$ cd mineola-1.3.0-Linux
root:/mineola-1.3.0-Linux$ cp -r * /usr/local/
```

Then in your project's own CMakeLists.txt, use
```cmake
find_package(mineola)
target_link_libraries(app PRIVATE mineola::mineola)
```

<!-- ## Build for iOS on MacOS

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
(p.s. the project targets x86 emulator by default, feel free to try changing the target arch/api_level) -->
