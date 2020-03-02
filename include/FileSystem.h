#ifndef MINEOLA_FILESYSTEM_H
#define MINEOLA_FILESYSTEM_H

#include <tuple>
#include <boost/algorithm/string.hpp>

namespace mineola { namespace file_system {

  std::string JoinPaths(const std::string &path0, const std::string &path1);
  std::tuple<std::string, std::string> SplitPath(const std::string &path);

  struct FileInfo {
    enum { kUnknown = 0, kFile = 1, kDir = 2 };
  };

  bool FileExists(const char *path);
  bool FileExists(const char *path, int &file_type);

}}

#endif /* MINEOLA_FILESYSTEM_H */
