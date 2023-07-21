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

## Build inside NVidia Cuda/GL docker

### Build a docker image with Mineola installed
For building inside NVidia GPU enabled containers (eg. nvcr.io/nvidia/cudagl), use or modify the provided Dockerfile.build.
For example, to build a docker image with headless EGL based server library installed, use the following commands:
```bash
$ docker build -f Dockerfile.build -t {tag} .
$ docker run --gpus=all -it -v .:/mineola {tag}
```
The headers and library will be installed in `/usr/local/include` and `/usr/local/lib`.

### Build Mineola in the container
To build and update the library inside the container, mount the repository directoy (eg. `/mineola-1.4.1'), and use the following command:
```bash
# inside docker
root:/$ cd mineola-1.4.1
root:/mineola-1.4.1$ cmake -S src -B build --preset=EGL
root:/mineola-1.4.1$ cmake --build build --target=install -j12
root:/mineola-1.4.1$ cd build
root:/mineola-1.4.1/build$ cpack
```
This will produce a zip archive containing all files need for deployment.

### Build app in NVidia Cuda/GL docker

Download from the release section or build your own as demonstrated above to get the mineola-1.4.1-Linux.zip archive.
To use the generated archive for app development, extract the contents into /usr/local:
```bash
# inside docker
root:/$ unzip mineola-1.4.1-Linux.zip
root:/$ cd mineola-1.4.1-Linux
root:/mineola-1.4.1-Linux$ cp -r * /usr/local/
```

Then in your project's own CMakeLists.txt, use
```cmake
find_package(mineola)
target_link_libraries(app PRIVATE mineola::mineola)
```

### Use official docker image
Directly build your app inside the mineola docker image:
```bash
$ docker run --gpus=all -it -v {app_dir}:/{app_dir} fyshen/mineola:1.4.1-egl-ubuntu20.04
```
Then use the cmake instructions to build your app.

## Build for iOS (on MacOS w/ Apple Silicon)

We use vcpkg and CMake to generate an XCode project for iOS development.
Install vcpkg and latest cmake, then:
```bash
$ mkdir build
$ cmake -S src -B build --preset=iOS\
> -DCMAKE_BUILD_TYPE={Debug|Release|MinSizeRel|RelWithDebInfo}  # default=Release
```
Use xcode to open the generated mineola.xcodeproj in the build directory.

<!-- ## Build for Android

Please use the included conanfile and CMake script to build the dynamic-link library libmineola.so for your target arch/api_level:
```bash
$ NDK=/path/to/ndk/build/cmake/android.toolchain.cmake conan create .. (user)/(channel) -s os=Android -s os.api_level=(min api) -s arch=(arch)
```

Then request the mineola shared library in your Android project by editing the build.gradle script.
Please see the included gltfview example Android Studio project for details.
(p.s. the project targets x86 emulator by default, feel free to try changing the target arch/api_level) -->
