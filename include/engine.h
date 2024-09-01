#include <vector>
#include <iostream>
#include "core.h"

#ifndef CENGINE_H
#define CENGINE_H

// # GameContext

class Node;

struct GameContext {
    std::vector<std::shared_ptr<Node>> nodes;
    float worldWidth;
    float worldHeight;
};

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
        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> nodes;

        Node(
            std::weak_ptr<Node> parent = std::weak_ptr<Node>()
        ) {
            this->parent = parent;
        }

        template <typename T>
        std::weak_ptr<T> AddNode(std::shared_ptr<T> node) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
            node->parent = shared_from_this();
            this->nodes.push_back(node);
            return node;
        }

        Node* RootNode() {
            if (auto pt = this->parent.lock()) {
                return pt->RootNode();
            }

            return this;
        }

        void TraverseNodeUpdate(GameContext* ctx) {
            this->Update(ctx);
            for (auto node: this->nodes) {
                node->TraverseNodeUpdate(ctx);
            }
        }

        void TraverseNodeRender(GameContext* ctx) {
            this->Render(ctx);
            for (auto node: this->nodes) {
                node->TraverseNodeRender(ctx);
            }
        }
};

#endif // CENGINE_H