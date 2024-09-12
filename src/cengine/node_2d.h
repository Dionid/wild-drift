#ifndef CENGINE_NODES_H
#define CENGINE_NODES_H

#include "node.h"

namespace cen {

class Node2D: public Node {
    public:
        Vector2 previousPosition;
        Vector2 position;
        int zOrder = 0;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Node2D::_tid;
        }

        Node2D(Vector2 position, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): Node(id, parent) {
            this->position = position;
            this->previousPosition = position;
            this->zOrder = zOrder;
        }

        Node2D* ClosestNode2DParent(Node* targetParent = nullptr) {
            auto currentParent = targetParent == nullptr ? this->parent : targetParent;
            if (currentParent == nullptr) {
                return nullptr;
            }

            auto n2dParent = dynamic_cast<Node2D*>(currentParent);
            if (n2dParent == nullptr) {
                if (currentParent->parent == nullptr) {
                    return nullptr;
                }

                return this->ClosestNode2DParent(currentParent->parent);
            }

            return n2dParent;
        }

        Vector2 GlobalPosition() {
            auto parent = this->ClosestNode2DParent();

            if (parent == nullptr) {
                return this->position;
            }

            return Vector2Add(parent->GlobalPosition(), this->position);
        }

        Vector2 PreviousGlobalPosition() {
            auto parent = this->ClosestNode2DParent();

            if (parent == nullptr) {
                return this->previousPosition;
            }

            return Vector2Add(parent->PreviousGlobalPosition(), this->previousPosition);
        }

        Node2D* RootNode2D() {
            auto p = this->ClosestNode2DParent();

            if (p == nullptr) {
                return this;
            }

            return p->RootNode2D();
        }

        void InvalidatePrevious() override {
            this->previousPosition = this->position;
        }
};

} // namespace cen

#endif // CENGINE_NODES_H