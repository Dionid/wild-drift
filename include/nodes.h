#include <raylib.h>
#include "engine.h"

#ifndef CENGINE_NODES_H
#define CENGINE_NODES_H

class Node2D: public Node {
    public:
        Vector2 position;
        Node2D(Vector2 position) {
            this->position = position;
        }
};

class CharacterBody2D: public Node2D {
    public:
        Size size;
        Vector2 velocity;
        CharacterBody2D(Vector2 position, Size size, Vector2 velocity = Vector2{}) : Node2D(position) {
            this->size = size;
            this->velocity = velocity;
        }
};

#endif // CENGINE_NODES_H