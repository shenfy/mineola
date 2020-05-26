#include <mineola/FileSystem.h>

#include <sys/stat.h>

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

namespace mineola { namespace file_system {

std::string JoinPaths(const std::string &path0, const std::string &path1) {
  if (path0.size() == 0
      || path0[path0.size() - 1] == '/'
      || path0[path0.size() - 1] == '\\') {
    return path0 + path1;
  } else {
    return path0 + '/' + path1;
  }
}

std::tuple<std::string, std::string> SplitPath(const std::string &path) {
  std::string folder;
  std::string fn = path;

  const auto pos = path.find_last_of("\\/");
  if (pos != std::string::npos) {
    // found
    folder = path.substr(0, pos + 1);
    if (pos < path.size() - 1) {
      fn = path.substr(pos + 1, path.size() - pos);
    } else {
      fn = "";
    }
  }
  return std::make_tuple(folder, fn);
}

bool FileExists(const char *path) {
  int file_type = 0;
  return FileExists(path, file_type);
}

bool FileExists(const char *path, int &file_type) {
  struct stat buffer;
  if (stat(path, &buffer) != -1) {
    if (S_ISDIR(buffer.st_mode)) {
      file_type = FileInfo::kDir;
      return true;
    } else if (S_ISREG(buffer.st_mode)) {
      file_type = FileInfo::kFile;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

}} //end namespace
