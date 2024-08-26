#include <raylib.h>
#include "engine.h"

#ifndef CENGINE_NODES_H
#define CENGINE_NODES_H

class Node2D: public Node {
    public:
        Vector2 position;
        Vector2 globalPosition;
        Node2D(Vector2 position, Vector2 globalPosition = Vector2{}, std::shared_ptr<Node> parent = nullptr): Node(parent) {
            this->position = position;
        }
};

std::shared_ptr<Node2D> findClosestNode2DParent(std::shared_ptr<Node> node) {
    if (node->parent == nullptr) {
        return nullptr;
    }

    auto parent = std::dynamic_pointer_cast<Node2D>(node->parent);

    if (parent != nullptr) {
        return parent;
    }

    return findClosestNode2DParent(parent);
}

void traverseNodeGlobalPosition(std::shared_ptr<Node> node, GameContext* ctx, GameObject* go) {
    auto currentNode = std::dynamic_pointer_cast<Node2D>(node);
    auto parent = findClosestNode2DParent(node);

    if (currentNode != nullptr && parent == nullptr) {
        currentNode.get()->globalPosition = currentNode.get()->position;
    } else if (currentNode != nullptr && parent != nullptr) {
        currentNode.get()->globalPosition = Vector2Add(
            currentNode.get()->position,
            parent.get()->globalPosition
        );
    }

    for (auto n: node->nodes) {
        traverseNodeGlobalPosition(n, ctx, go);
    }
}

void traverseGameObjectGlobalPosition(GameObject* go, GameContext* ctx) {
    traverseNodeGlobalPosition(
        go->rootNode,
        ctx,
        go
    );
}

class CharacterBody2D: public Node2D {
    public:
        Size size;
        Vector2 velocity;
        CharacterBody2D(Vector2 position, Size size, Vector2 velocity = Vector2{},  Vector2 globalPosition = Vector2{}, std::shared_ptr<Node> parent = nullptr) : Node2D(position, globalPosition, parent) {
            this->size = size;
            this->velocity = velocity;
        }
};

#endif // CENGINE_NODES_H