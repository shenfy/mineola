#ifndef MINEOLA_ENTITY_H
#define MINEOLA_ENTITY_H

#include <boost/uuid/uuid.hpp>

namespace mineola {

class Entity {
public:
  Entity();
  virtual ~Entity();

  const boost::uuids::uuid &Id();

  // life cycle
  virtual void Start();
  virtual void FrameMove(double time, double frame_time);
  virtual void PreRender();
  virtual void PostRender();
  virtual void Destroy();

protected:
  const boost::uuids::uuid id_;
};

} //namespace


#endif