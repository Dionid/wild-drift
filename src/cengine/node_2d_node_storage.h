#ifndef CENGINE_NODE_2D_NODE_STORAGE_H_
#define CENGINE_NODE_2D_NODE_STORAGE_H_

#include "node_2d.h"
#include "node_storage.h"

namespace cen {

inline void Node2D::setZOrder(int zOrder) {
    this->zOrder = zOrder;
    this->storage->SortRenderNodes();
}

}

#endif // CENGINE_NODE_2D_NODE_STORAGE_H_