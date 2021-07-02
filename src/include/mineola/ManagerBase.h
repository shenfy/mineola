#ifndef MINEOLA_MANAGERBASE_H
#define MINEOLA_MANAGERBASE_H

#include <memory>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace mineola {

template <typename T, typename TPtr = std::shared_ptr<T> >
class ManagerBase {
public:
  ManagerBase();
  virtual ~ManagerBase();

  TPtr &Find(const std::string &name);
  const TPtr &Find(const std::string &name) const;
  std::string QueryName(const TPtr &ptr) const;
  void Remove(const std::string &name);
  void Add(const std::string &name, TPtr &pObj);
  void Add(const std::string &name, TPtr &&pObj); //overload for rvalue reference

  template <typename TT>
  void Traverse(TT &visitor);

  template <typename TT>
  void Transform(const TT &op);

  void ReleaseResources();
  virtual void Release();

protected:
  std::unordered_map<std::string, TPtr> map_;
};

template <typename T, typename TPtr>
ManagerBase<T, TPtr>::ManagerBase() {
}

template <typename T, typename TPtr>
ManagerBase<T, TPtr>::~ManagerBase() {
  map_.clear();
}

template <typename T, typename TPtr>
TPtr &ManagerBase<T, TPtr>::Find(const std::string &name) {
  static TPtr pNullObj;
  auto iter = map_.find(name);
  if (iter != map_.end()) //found
    return iter->second;
  else
    return pNullObj;
}

template <typename T, typename TPtr>
const TPtr &ManagerBase<T, TPtr>::Find(const std::string &name) const {
  const static TPtr pNullObj;
  auto iter = map_.find(name);
  if (iter != map_.end()) //found
    return iter->second;
  else
    return pNullObj;
}

template <typename T, typename TPtr>
std::string ManagerBase<T, TPtr>::QueryName(const TPtr &ptr) const {
  auto iter = std::find_if(map_.begin(), map_.end(), [&ptr](const auto &kvp) {
    return kvp.second == ptr;
  });
  if (iter != map_.end())
    return iter->first;
  else
    return "";
}


template <typename T, typename TPtr>
void ManagerBase<T, TPtr>::Add(const std::string &name, TPtr &pObj) {
  map_[name] = pObj;
}

template <typename T, typename TPtr>
void ManagerBase<T, TPtr>::Add(const std::string &name, TPtr &&pObj) {
  map_[name] = pObj;
}

template <typename T, typename TPtr>
void ManagerBase<T, TPtr>::Remove(const std::string &name) {
  auto iter = map_.find(name);
  if (iter != map_.end())
    map_.erase(iter);
}

template <typename T, typename TPtr> template <typename TT>
void ManagerBase<T, TPtr>::Traverse(TT &visitor) {
  for (auto iter = map_.begin(); iter != map_.end(); ++iter)
    visitor(iter->first, iter->second);
}

template <typename T, typename TPtr> template <typename TT>
void ManagerBase<T, TPtr>::Transform(const TT &op) {
  for (auto iter = map_.begin(); iter != map_.end(); ++iter) {
    op(iter->first, iter->second);
  }
}

template <typename T, typename TPtr>
void ManagerBase<T, TPtr>::ReleaseResources() {
  map_.clear();
}

template <typename T, typename TPtr>
void ManagerBase<T, TPtr>::Release() {
  ReleaseResources();
}


} //namespaces

#endif
