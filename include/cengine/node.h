#ifndef CENGINE_NODE_H_
#define CENGINE_NODE_H_

#include <vector>
#include <iostream>
#include "core.h"
#include "game_context.h"

// # Renderer and Updater

class Renderer {
    public:
        virtual void Render(GameContext* ctx) {};
};

class Updater {
    public:
        virtual void Update(GameContext* ctx) {};
};

// # Node

class Node: public Renderer, public Updater, public WithType {
    public:
        Node* parent;
        std::vector<std::unique_ptr<Node>> children;

        static const uint64_t _id;

        uint64_t TypeId() const override {
            return Node::_id;
        }

        Node(
            Node* parent = nullptr
        ) {
            this->parent = parent;
        }

        template <typename T>
        T* AddNode(std::unique_ptr<T> node) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
            node->parent = this;
            auto ptr = node.get();
            this->children.push_back(std::move(node));
            return ptr;
        }

        template <typename T>
        std::vector<Node*> GetByType() {
            std::vector<Node*> nodes;

            for (const auto& node: this->children) {
                if (dynamic_cast<T*>(node.get())) {
                    nodes.push_back(node.get());
                }
            }

            return nodes;
        }

        template <typename T>
        Node* GetFirstByType() {
            for (const auto& node: this->children) {
                if (dynamic_cast<T*>(node.get())) {
                    return node.get();
                }
            }

            return nullptr;
        }

        void RemoveNode(Node* node) {
            for (auto it = this->children.begin(); it != this->children.end(); it++) {
                if (it->get() == node) {
                    this->children.erase(it);
                    return;
                }
            }
        }

        Node* RootNode() {
            if (this->parent != nullptr) {
                return this->parent->RootNode();
            }

            return this;
        }

        void TraverseNodeUpdate(GameContext* ctx) {
            this->Update(ctx);
            for (const auto& node: this->children) {
                node->TraverseNodeUpdate(ctx);
            }
        }

        void TraverseNodeRender(GameContext* ctx) {
            this->Render(ctx);
            for (const auto& node: this->children) {
                node->TraverseNodeRender(ctx);
            }
        }
};

#endif // CENGINE_NODE_H_