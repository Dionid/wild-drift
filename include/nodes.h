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

        Vector2 GetGlobalPosition() {
            if (this->parent == nullptr) {
                return this->position;
            }

            auto parent = this->FindClosestNode2DParent();

            if (parent == nullptr) {
                return this->position;
            }

            return Vector2Add(parent->GetGlobalPosition(), this->position);
        }

        std::shared_ptr<Node2D> FindClosestNode2DParent() {
            if (this->parent == nullptr) {
                return nullptr;
            }

            auto parent = std::dynamic_pointer_cast<Node2D>(this->parent);

            if (parent != nullptr) {
                return parent;
            }

            return parent->FindClosestNode2DParent();
        }

        Node2D* GetRootNode2D() {
            if (this->parent == nullptr) {
                return this;
            }

            auto p = this->FindClosestNode2DParent();

            if (p == nullptr) {
                return this;
            }

            return p->GetRootNode2D();
        }
};

#endif // CENGINE_NODES_H