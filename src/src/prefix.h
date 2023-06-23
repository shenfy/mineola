#ifndef MINEOLA_PREFIX_H
#define MINEOLA_PREFIX_H

#include <mineola/GLMDefines.h>

#ifdef __ANDROID_API__
#define BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX
#endif

#define MINEOLA_USE_STBIMAGE

#endif  /* MINEOLA_PREFIX_H */
