#ifndef CENGINE_NODE_NODE_STORAGE_H_
#define CENGINE_NODE_NODE_STORAGE_H_

#include "node.h"
#include "node_storage.h"

namespace cen {

template <typename T>
T* Node::AddNode(std::unique_ptr<T> newNode) {
    static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
    if (this->storage == nullptr) {
        throw std::runtime_error("Node storage is not set");
    }
    newNode->storage = this->storage;
    newNode->parent = this;
    newNode->scene = this->scene;
    auto nodePtr = newNode.get();
    if (nodePtr->id == 0) {
        nodePtr->id = NodeIdGenerator::GetInstance().GetNextId();
    }
    this->children.push_back(std::move(newNode));
    this->storage->OnNestedNodeCreated(nodePtr);
    return nodePtr;
}

inline void Node::RemoveChild(Node* node) {
    this->storage->RemoveFromIndex(node);

    for (auto i = 0; i < this->children.size(); i++) {
        if (this->children[i].get() == node) {
            this->children.erase(this->children.begin() + i);
            break;
        }
    }
}

inline void Node::RemoveChildById(node_id_t id) {
    this->storage->RemoveFromIndexById(id);

    for (auto i = 0; i < this->children.size(); i++) {
        if (this->children[i]->id == id) {
            this->children.erase(this->children.begin() + i);
            break;
        }
    }
}

} // namespace cen

#endif // CENGINE_NODE_NODE_STORAGE_H_