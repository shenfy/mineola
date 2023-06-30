#ifndef MINEOLA_GLUTILITY_H
#define MINEOLA_GLUTILITY_H

#include <cstdint>
#include <cstdio>
#include <chrono>
#include <memory>

#ifdef __ANDROID_API__
  #include <GLES3/gl3.h>
  #include <GLES3/gl3ext.h>
  #include <android/log.h>
#elif defined(__APPLE__)
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR // iOS
    #include <OpenGLES/ES3/glext.h>
  // #elif TARGET_OS_OSX  // macOS
  //   #include "glad.h"
  //   #define MINEOLA_DESKTOP
  #endif
#elif defined(__linux__) || defined(_WIN32)  // desktop
  #include "glad.h"
  #define MINEOLA_DESKTOP
#endif

namespace mineola {

inline const char *gluGetErrorString(uint32_t error) {
  switch (error) {
  case GL_NO_ERROR: return 0;
  case GL_INVALID_ENUM: return "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
  case GL_INVALID_VALUE: return "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
  case GL_INVALID_OPERATION: return "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
  case GL_INVALID_FRAMEBUFFER_OPERATION: return "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
  case GL_OUT_OF_MEMORY: return "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
  default: return "Unknown error code!";
  }
}

inline const char *getFramebufferStatusString(uint32_t status) {
  switch (status) {
  case GL_FRAMEBUFFER_COMPLETE: return 0;
  case GL_FRAMEBUFFER_UNDEFINED: return "Framebuffer undefined";
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "Framebuffer incomplete attachment";
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "Framebuffer incomplete missing attachment";
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: return "Framebuffer incomplete dimensions";
  case GL_FRAMEBUFFER_UNSUPPORTED: return "Framebuffer unsupported";
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "Framebuffer incomplete multisample";
  default: return "Unknown framebuffer status code";
  }
}

//Timer class use mili-second (1e-3 second) representation
class Timer {
public:
  Timer() {}
  ~Timer() {}

  typedef std::chrono::duration<float, std::milli> milli_sec_type;

  inline void Start() {
    start_time_ = std::chrono::steady_clock::now();
  }

  inline double Snapshot() const {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<milli_sec_type>(now - start_time_).count();
  }

  std::chrono::steady_clock::time_point start_time_;
};

template <typename T0, typename T1>
inline std::shared_ptr<T0> bd_cast(T1 &p) {
  return std::dynamic_pointer_cast<T0>(p);
}

}


#ifdef __ANDROID_API__

#define MLOG(...) __android_log_print(ANDROID_LOG_VERBOSE, "Mineola", __VA_ARGS__)
#define CHKGLERR {const char *p = gluGetErrorString(glGetError()); if (p) {__android_log_print(ANDROID_LOG_VERBOSE, "Error", "%s:%d, %s\n", __FILE__, __LINE__, p);} }
#define CHKGLERR_RET {const char *p = gluGetErrorString(glGetError()); if (p) {__android_log_print(ANDROID_LOG_VERBOSE, "Error", "%s:%d, %s\n", __FILE__, __LINE__, p); return false;}}

#else  // not android

#define MLOG(...) printf(__VA_ARGS__);
#define CHKGLERR {const char *p = gluGetErrorString(glGetError()); if (p) {printf("Error: %s:%d, %s\n", __FILE__, __LINE__, p);} }
#define CHKGLERR_RET {const char *p = gluGetErrorString(glGetError()); if (p) {printf("Error: %s:%d, %s\n", __FILE__, __LINE__, p); return false;}}

#endif  // android

#endif
