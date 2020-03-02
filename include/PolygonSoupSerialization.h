#ifndef MINEOLA_POLYGONSOUPSERIALIZATION_H
#define MINEOLA_POLYGONSOUPSERIALIZATION_H

#include "PolygonSoup.h"

namespace mineola {
  bool SerializeSoup(const char *filename, const PolygonSoup &soup, bool is_binary);
  bool DeserializeSoup(const char *filename, bool is_binary);

  bool WriteSoupToPLY(const char *filename, const PolygonSoup &soup);
  bool LoadSoupFromPLY(const char *filename, PolygonSoup &soup);
  bool LoadSoupFromPLY(std::istream &ins, PolygonSoup &soup);
}

#endif  // MINEOLA_POLYGONSOUPSERIALIZATION_H
