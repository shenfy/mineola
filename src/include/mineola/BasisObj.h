#ifndef MINEOLA_BASICOBJ_H
#define MINEOLA_BASICOBJ_H

#include "Visitor.h"

namespace mineola {

class Resource : Visitable<> {
public:
  Resource();
  virtual ~Resource();
  MINEOLA_VISITOR_ACCEPT_FUNC
};

} //namespace

#endif
