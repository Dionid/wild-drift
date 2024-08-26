#include <vector>
#include <iostream>

#ifndef CENGINE_H
#define CENGINE_H

// # Core

class Node;

class GameObject {
    public:
        std::shared_ptr<Node> rootNode;
        GameObject(std::shared_ptr<Node> rootNode) {
            this->rootNode = rootNode;
        }
        // ~GameObject() {}
        GameObject(const GameObject& other) {
            this->rootNode = other.rootNode;
        }
        GameObject& operator=(const GameObject& other) {
            this->rootNode = other.rootNode;
            return *this;
        }
};

struct GameContext {
    std::vector<GameObject*> gos;
    float worldWidth;
    float worldHeight;
};

class Renderer {
    public:
        virtual void Render(GameContext* ctx, GameObject* thisGO) {};
};

class Updater {
    public:
        virtual void Update(GameContext* ctx, GameObject* thisGO) {};
};

class Node: public Renderer, public Updater, public std::enable_shared_from_this<Node> {
    public:
        std::vector<std::shared_ptr<Node>> nodes;
        std::shared_ptr<Node> parent;
        Node(
            std::shared_ptr<Node> parent = nullptr
        ) {
            this->parent = parent;
        }
        // ~Node() {}
        Node(const Node& other) {
            this->nodes = other.nodes;
            this->parent = other.parent;
        }
        Node& operator=(const Node& other) {
            this->nodes = other.nodes;
            this->parent = other.parent;
            return *this;
        }
        void AddNode(std::shared_ptr<Node> node) {
            node->parent = shared_from_this();
            this->nodes.push_back(node);
        }
};

// # Traverse

void NodeTraverseUpdate(std::shared_ptr<Node> node, GameContext* ctx, GameObject* go) {
    node->Update(ctx, go);
    for (auto n: node->nodes) {
        NodeTraverseUpdate(n, ctx, go);
    }
}

void GameObjectTraverseUpdate(GameObject* go, GameContext* ctx) {
    NodeTraverseUpdate(
        go->rootNode,
        ctx,
        go
    );
}

void NodeTraverseRender(std::shared_ptr<Node> node, GameContext* ctx, GameObject* go) {
    node->Render(ctx, go);
    for (auto n: node->nodes) {
        NodeTraverseRender(n, ctx, go);
    }
}

void GameObjectTraverseRender(GameObject* go, GameContext* ctx) {
    NodeTraverseRender(
        go->rootNode,
        ctx,
        go
    );
}

// # Types

struct Size {
    float width;
    float height;
};

// # Delta
const float FPS = 60.0f;
const float secondsPerFrame = 1.0f / FPS;

float DeltaTime() {
    return GetFrameTime() / secondsPerFrame;
}

#endif // CENGINE_H