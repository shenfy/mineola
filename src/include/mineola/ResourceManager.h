#ifndef MINEOLA_RESRCMGR_H
#define MINEOLA_RESRCMGR_H

#include <vector>
#include <string>
#include "ManagerBase.h"
#include "BasisObj.h"

namespace mineola {

class ResourceManager : public ManagerBase<Resource> {
public:
  ResourceManager();

  void AddSearchPath(const char *path);
  bool LocateFile(const char *filename, std::string &found_path);
  void PopSearchPath(const char *path = nullptr);
  void Release() override;

protected:
  std::vector<std::string> paths_;
};

}

#endif
