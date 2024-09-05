#include "node.h"
#include "node_storage.h"

template <typename T>
T* Node::AddNode(std::unique_ptr<T> node) {
    static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
    if (this->storage == nullptr) {
        throw std::runtime_error("Node storage is not set");
    }
    node->storage = this->storage;
    node->parent = this;
    auto nodePtr = node.get();
    if (nodePtr->id == 0) {
        nodePtr->id = NodeIdGenerator::GetInstance().GetNextId();
    }
    this->children.push_back(std::move(node));
    this->storage->OnNestedNodeCreated(nodePtr);
    return nodePtr;
}