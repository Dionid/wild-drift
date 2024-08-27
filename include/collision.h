#include <raymath.h>
#include "core.h"
#include "nodes.h"
#include "engine.h"

#ifndef CENGINE_COLLISION_H
#define CENGINE_COLLISION_H

enum class ColliderType {
    Solid,
    Sensor
};

struct ShapeRectangle {
    Size size;
};

struct ShapeCircle {
    float radius;
};

class Shape {
    public:
        enum Type {
            RECTANGLE = 0,
            CIRCLE = 1
        } type;

        union {
            ShapeRectangle rect;
            ShapeCircle circle;
        };

    static Shape Rectangle(Size size) {
        return Shape {
            .type = Type::RECTANGLE,
            .rect = { size }
        };
    }

    static Shape Circle(float radius) {
        return Shape {
            .type = Type::CIRCLE,
            .circle = { radius }
        };
    }
};

class Collider: public Node2D {
    public:
        ColliderType type;
        Shape shape;

        Collider(ColliderType type, Shape shape, Vector2 position, std::shared_ptr<Node> parent = nullptr): Node2D(position, parent) {
            this->type = type;
            this->shape = shape;
        }
};

struct CollisionHit {
    float penetration;
    Vector2 normal;
};

CollisionHit CircleRectangleCollision(
    Vector2 circlePosition,
    float circleRadius,
    Vector2 rectPosition,
    Size rectSize
) {
    Vector2 closest = {
        std::clamp(circlePosition.x, rectPosition.x - rectSize.width/2, rectPosition.x + rectSize.width/2),
        std::clamp(circlePosition.y, rectPosition.y - rectSize.height/2, rectPosition.y + rectSize.height/2)
    };

    Vector2 distance = Vector2Subtract(circlePosition, closest);
    float distanceLength = Vector2Length(distance);

    if (distanceLength < circleRadius) {
        return {
            circleRadius - distanceLength,
            Vector2Normalize(distance)
        };
    }

    return {
        0,
        Vector2{}
    };
}

#endif //CENGINE_COLLISION_H