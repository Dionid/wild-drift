#include <raylib.h>
#include "engine.h"

#ifndef CENGINE_NODES_H
#define CENGINE_NODES_H

class Node2D: public Node {
    public:
        Vector2 position;

        Node2D(Vector2 position, std::shared_ptr<Node> parent = nullptr): Node(parent) {
            this->position = position;
        }

        Vector2 GlobalPosition() {
            if (this->parent == nullptr) {
                return this->position;
            }

            auto parent = this->ClosestNode2DParent();

            if (parent == nullptr) {
                return this->position;
            }

            return Vector2Add(parent->GlobalPosition(), this->position);
        }

        std::shared_ptr<Node2D> ClosestNode2DParent() {
            if (this->parent == nullptr) {
                return nullptr;
            }

            auto parent = std::dynamic_pointer_cast<Node2D>(this->parent);

            if (parent != nullptr) {
                return parent;
            }

            return parent->ClosestNode2DParent();
        }

        Node2D* RootNode2D() {
            if (this->parent == nullptr) {
                return this;
            }

            auto p = this->ClosestNode2DParent();

            if (p == nullptr) {
                return this;
            }

            return p->RootNode2D();
        }
};

#endif // CENGINE_NODES_H