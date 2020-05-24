#ifndef MINEOLA_PREFIX_H
#define MINEOLA_PREFIX_H

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CTOR_INIT

#ifdef __ANDROID_API__
#define BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX
#endif

#if defined(__APPLE__)
  #include <TargetConditionals.h>
  #if TARGET_IPHONE_SIMULATOR
    // iOS Simulator
  #elif TARGET_OS_IPHONE
    // iOS device
  #else  // MacOS or others
    #define MINEOLA_USE_STBIMAGE
  #endif
#else
  #define MINEOLA_USE_STBIMAGE
#endif

#endif /* MINEOLA_PREFIX_H */
