#include "node.h"

const uint64_t Node::_tid = TypeIdGenerator::getInstance().getNextId();

Node* Node::RootNode() {
    if (this->parent != nullptr) {
        return this->parent->RootNode();
    }

    return this;
};

void Node::TraverseUpdate(GameContext* ctx) {
    this->Update(ctx);
    for (const auto& node: this->children) {
        node->TraverseUpdate(ctx);
    }
};

// template <typename T>
// T* Node::AddNode(std::unique_ptr<T> node) {
//     static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
//     node->storage = this->storage;
//     node->parent = this;
//     auto ptr = node.get();
//     if (ptr->id == 0) {
//         ptr->id = NodeIdGenerator::GetInstance().GetNextId();
//     }
//     this->children.push_back(std::move(node));
//     return ptr;
// };
