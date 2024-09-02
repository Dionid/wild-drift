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

class Node: public Renderer, public Updater, public enable_shared_from_base<Node> {
    public:
        
        Node* parent;
        std::vector<std::unique_ptr<Node>> nodes;

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
            this->nodes.push_back(std::move(node));
            return ptr;
        }

        // TODO: RemoveNode

        Node* RootNode() {
            if (this->parent != nullptr) {
                return this->parent->RootNode();
            }

            return this;
        }

        void TraverseNodeUpdate(GameContext* ctx) {
            this->Update(ctx);
            for (const auto& node: this->nodes) {
                node->TraverseNodeUpdate(ctx);
            }
        }

        void TraverseNodeRender(GameContext* ctx) {
            this->Render(ctx);
            for (const auto& node: this->nodes) {
                node->TraverseNodeRender(ctx);
            }
        }
};

#endif // CENGINE_NODE_H_