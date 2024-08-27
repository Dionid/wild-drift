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
        std::vector<std::shared_ptr<Node>> nodes;
        std::shared_ptr<Node> parent;

        Node(
            std::shared_ptr<Node> parent = nullptr
        ) {
            this->parent = parent;
        }

        std::shared_ptr<Node> AddNode(std::shared_ptr<Node> node) {
            node->parent = shared_from_this();
            this->nodes.push_back(node);
            return node;
        }

        Node* GetRootNode() {
            if (this->parent == nullptr) {
                return this;
            }

            return this->parent->GetRootNode();
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

// # Delta
const float FPS = 60.0f;
const float secondsPerFrame = 1.0f / FPS;

float DeltaTime() {
    return GetFrameTime() / secondsPerFrame;
}

#endif // CENGINE_H