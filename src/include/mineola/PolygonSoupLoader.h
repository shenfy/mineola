#ifndef MINEOLA_POLYGONSOUPLOADER_H
#define MINEOLA_POLYGONSOUPLOADER_H

// #include "Renderable.h"

namespace mineola {

struct PolygonSoup;
class Renderable;

namespace primitive_helper {

bool BuildFromPolygonSoup(const PolygonSoup &soup,
  const char *name, Renderable &renderable);

}}  // end namespace

#endif  // MINEOLA_POLYGONSOUPLOADER_H
