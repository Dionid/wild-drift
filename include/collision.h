#include <memory>
#include <raymath.h>
#include "core.h"
#include "engine.h"
#include "node-2d.h"

#ifndef CENGINE_COLLISION_H
#define CENGINE_COLLISION_H

// # Collision

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

// # Nodes

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

// # ColliderBody

class CollisionObject2D;

struct Collision {
    CollisionHit hit;
    std::shared_ptr<Collider> selfCollider;
    std::shared_ptr<CollisionObject2D> other;
};

class CollisionObject2D: public Node2D {
    public:
        CollisionObject2D(Vector2 position, std::shared_ptr<Node> parent = nullptr): Node2D(position, parent) {}
        virtual void OnCollision(Collision c) {}

        virtual std::shared_ptr<CollisionObject2D> GetCollisionObject2DShared() {
            return Node::downcasted_shared_from_this<CollisionObject2D>();
        }
};

// # Checks

void collisionCheck(
    GameContext* ctx
) {
    for (auto i = 0; i < ctx->nodes.size(); i++) {
        auto node = ctx->nodes[i];

        auto co = dynamic_pointer_cast<CollisionObject2D>(node);
        if (co == nullptr) {
            continue;
        }

        for (auto n: node->nodes) {
            auto collider = dynamic_pointer_cast<Collider>(n);

            if (collider == nullptr) {
                continue;
            }

            for (auto j = i + 1; j < ctx->nodes.size(); j++) {
                auto otherNode = ctx->nodes[j];

                if (otherNode == node) {
                    continue;
                }

                auto otherCo = dynamic_pointer_cast<CollisionObject2D>(otherNode);
                if (otherCo == nullptr) {
                    continue;
                }

                for (auto on: otherNode->nodes) {
                    auto otherCollider = dynamic_pointer_cast<Collider>(on);

                    if (otherCollider == nullptr) {
                        continue;
                    }

                    auto collision = CollisionHit{0, Vector2{}};

                    switch (collider->shape.type) {
                        case Shape::Type::RECTANGLE:
                            switch (otherCollider->shape.type) {
                                case Shape::Type::RECTANGLE:
                                    break;
                                case Shape::Type::CIRCLE:
                                    collision = CircleRectangleCollision(
                                        otherCollider->GetGlobalPosition(),
                                        otherCollider->shape.circle.radius,
                                        collider->GetGlobalPosition(),
                                        collider->shape.rect.size
                                    );
                                    break;
                            }
                            break;
                        case Shape::Type::CIRCLE:
                            switch (otherCollider->shape.type) {
                                case Shape::Type::RECTANGLE:
                                    collision = CircleRectangleCollision(
                                        collider->GetGlobalPosition(),
                                        collider->shape.circle.radius,
                                        otherCollider->GetGlobalPosition(),
                                        otherCollider->shape.rect.size
                                    );
                                    break;
                                case Shape::Type::CIRCLE:
                                    break;
                            }
                            break;
                    }

                    if (collision.penetration > 0) {
                        co->OnCollision({
                            collision,
                            collider,
                            otherCo
                        });
                        otherCo->OnCollision({
                            collision,
                            otherCollider,
                            co
                        });
                    }
                }
            } 
        }
    }
}

#endif //CENGINE_COLLISION_H