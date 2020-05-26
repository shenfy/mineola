#include "prefix.h"
#include "../include/ResourceManager.h"
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#include "../include/FileSystem.h"
#include "../include/glutility.h"

namespace {

bool IsAbsolutePath(const char *path) {
  if (path[0] == '/') {
    return true;
  }

#ifdef _WIN32
  if (path[0] == '\\' ||
    (path[0] >= 'A' && path[0] <= 'Z' || path[0] >= 'a' && path[0] <= 'z') && path[1] == ':') {
    return true;
  }
#endif

  return false;
}

}

namespace mineola {

ResourceManager::ResourceManager() {
}

void ResourceManager::AddSearchPath(const char *path) {
  auto iter = std::find(paths_.begin(), paths_.end(), path);
  if (iter == paths_.end()) {
    paths_.push_back(path);
    MLOG("path added: %s\n", path);
  }
}

bool ResourceManager::LocateFile(const char *filename, std::string &found_path) {
  if (filename == nullptr) {
    return false;
  }

  if (IsAbsolutePath(filename)) {
    if (file_system::FileExists(filename)) {
      found_path = filename;
      return true;
    }
  } else {
    for (auto &path : paths_) {
      std::string file_path = file_system::JoinPaths(path, filename);
      if (file_system::FileExists(file_path.c_str())) {
        found_path = file_path;
        return true;
      }
    }
  }

  return false;
}

void ResourceManager::PopSearchPath(const char *path) {
  if (paths_.size() == 0) {
    return;
  }

  if (path == nullptr) {
    paths_.pop_back();
  } else {
    auto iter = std::find(paths_.rbegin(), paths_.rend(), path);
    if (iter != paths_.rend()) {
      paths_.erase(iter.base() - 1);
    }
  }
}

}
