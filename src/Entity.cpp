#include "prefix.h"
#include "../include/Entity.h"
#include <boost/uuid/uuid_generators.hpp>

namespace mineola {

Entity::Entity() : id_(boost::uuids::random_generator()()) {
}

Entity::~Entity() {
}

const boost::uuids::uuid &Entity::Id() {
	return id_;
}

void Entity::Start() {}
void Entity::FrameMove(double time, double frame_time) {}
void Entity::PreRender() {}
void Entity::PostRender() {}
void Entity::Destroy() {}

}