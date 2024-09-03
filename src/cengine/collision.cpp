#include "cengine/scene.h"
#include "cengine/collision.h"

const uint64_t Collider::_tid = TypeIdGenerator::getInstance().getNextId();

const uint64_t CollisionObject2D::_tid = TypeIdGenerator::getInstance().getNextId();