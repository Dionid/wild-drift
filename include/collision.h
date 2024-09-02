#ifndef CENGINE_COLLISION_H
#define CENGINE_COLLISION_H

#include <memory>
#include <raymath.h>
#include "core.h"
#include "engine.h"
#include "node-2d.h"

// # Collision

struct CollisionHit {
    float penetration;
    Vector2 normal;
};

static CollisionHit CircleRectangleCollision(
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

static CollisionHit RectangleRectangleCollision(
    Vector2 positionA,
    Size sizeA,
    Vector2 positionB,
    Size sizeB
) {
    Vector2 distance = Vector2Subtract(positionA, positionB);

    Vector2 halfSizeA = {
        sizeA.width/2,
        sizeA.height/2
    };

    Vector2 halfSizeB = {
        sizeB.width/2,
        sizeB.height/2
    };

    Vector2 overlap = Vector2Subtract(
        Vector2Add(halfSizeA, halfSizeB),
        Vector2Abs(distance)
    );

    if (overlap.x > 0 && overlap.y > 0) {
        return {
            std::min(overlap.x, overlap.y),
            Vector2Normalize(distance)
        };
    }

    return {
        0,
        Vector2{}
    };
}

static CollisionHit CircleCircleCollision(
    Vector2 positionA,
    float radiusA,
    Vector2 positionB,
    float radiusB
) {
    Vector2 distance = Vector2Subtract(positionA, positionB);

    float distanceLength = Vector2Length(distance);

    if (distanceLength < radiusA + radiusB) {
        return {
            radiusA + radiusB - distanceLength,
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
    Collider* selfCollider;
    CollisionObject2D* other;
};

class CollisionObject2D: public Node2D {
    public:
        CollisionObject2D(Vector2 position, std::shared_ptr<Node> parent = nullptr): Node2D(position, parent) {}
        virtual void OnCollision(Collision c) {}
        virtual void OnCollisionStarted(Collision c) {}
        virtual void OnCollisionEnded(Collision c) {}
};

// # Checks

struct CollisionEvent {
    CollisionHit hit;
    CollisionObject2D* collisionObjectA;
    Collider* colliderA;
    CollisionObject2D* collisionObjectB;
    Collider* colliderB;
};

class CollisionEngine {
    public:
        std::vector<CollisionEvent> collisionsEventVec;

        CollisionEngine() {
            this->collisionsEventVec = std::vector<CollisionEvent>();
        }

        void NarrowCollisionCheckNaive(
            GameContext* ctx
        ) {
            std::vector<CollisionEvent> currentCollisions;

            for (auto i = 0; i < ctx->nodes.size(); i++) {
                auto node = ctx->nodes[i];

                auto co = std::dynamic_pointer_cast<CollisionObject2D>(node);
                if (co == nullptr) {
                    continue;
                }

                for (const auto& n: node->nodes) {
                    auto collider = dynamic_cast<Collider*>(n.get());

                    if (collider == nullptr) {
                        continue;
                    }

                    for (auto j = i + 1; j < ctx->nodes.size(); j++) {
                        auto otherNode = ctx->nodes[j];

                        if (otherNode == node) {
                            continue;
                        }

                        auto otherCo = std::dynamic_pointer_cast<CollisionObject2D>(otherNode);
                        if (otherCo == nullptr) {
                            continue;
                        }

                        for (const auto& on: otherNode->nodes) {
                            auto otherCollider = dynamic_cast<Collider*>(on.get());

                            if (otherCollider == nullptr) {
                                continue;
                            }

                            auto collision = CollisionHit{0, Vector2{}};

                            switch (collider->shape.type) {
                                case Shape::Type::RECTANGLE:
                                    switch (otherCollider->shape.type) {
                                        case Shape::Type::RECTANGLE:
                                            collision = RectangleRectangleCollision(
                                                collider->GlobalPosition(),
                                                collider->shape.rect.size,
                                                otherCollider->GlobalPosition(),
                                                otherCollider->shape.rect.size
                                            );
                                            break;
                                        case Shape::Type::CIRCLE:
                                            collision = CircleRectangleCollision(
                                                otherCollider->GlobalPosition(),
                                                otherCollider->shape.circle.radius,
                                                collider->GlobalPosition(),
                                                collider->shape.rect.size
                                            );
                                            break;
                                    }
                                    break;
                                case Shape::Type::CIRCLE:
                                    switch (otherCollider->shape.type) {
                                        case Shape::Type::RECTANGLE:
                                            collision = CircleRectangleCollision(
                                                collider->GlobalPosition(),
                                                collider->shape.circle.radius,
                                                otherCollider->GlobalPosition(),
                                                otherCollider->shape.rect.size
                                            );
                                            break;
                                        case Shape::Type::CIRCLE:
                                            collision = CircleCircleCollision(
                                                collider->GlobalPosition(),
                                                collider->shape.circle.radius,
                                                otherCollider->GlobalPosition(),
                                                otherCollider->shape.circle.radius
                                            );
                                            break;
                                    }
                                    break;
                            }

                            if (collision.penetration > 0) {
                                currentCollisions.push_back({
                                    collision,
                                    co.get(),
                                    collider,
                                    otherCo.get(),
                                    otherCollider
                                });
                            }
                        }
                    } 
                }
            }

            std::vector<CollisionEvent> newCollisions;

            for (auto collision: currentCollisions) {
                bool found = false;

                for (auto oldCollision: this->collisionsEventVec) {
                    if (
                        (oldCollision.collisionObjectA == collision.collisionObjectB &&
                        oldCollision.collisionObjectB == collision.collisionObjectA) || 
                        (oldCollision.collisionObjectA == collision.collisionObjectA &&
                        oldCollision.collisionObjectB == collision.collisionObjectB)
                    ) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    newCollisions.push_back(collision);
                }
            }

            std::vector<CollisionEvent> endedCollisions;

            for (auto oldCollision: this->collisionsEventVec) {
                bool found = false;

                for (auto collision: currentCollisions) {
                    if (
                        (oldCollision.collisionObjectA == collision.collisionObjectB &&
                        oldCollision.collisionObjectB == collision.collisionObjectA) || 
                        (oldCollision.collisionObjectA == collision.collisionObjectA &&
                        oldCollision.collisionObjectB == collision.collisionObjectB)
                    ) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    endedCollisions.push_back(oldCollision);
                }
            }

            for (auto collision: newCollisions) {
                collision.collisionObjectA->OnCollisionStarted({
                    collision.hit,
                    collision.colliderA,
                    collision.collisionObjectB,
                });
                collision.collisionObjectB->OnCollisionStarted({
                    collision.hit,
                    collision.colliderB,
                    collision.collisionObjectA,
                });
            }

            for (auto collision: currentCollisions) {
                collision.collisionObjectA->OnCollision({
                    collision.hit,
                    collision.colliderA,
                    collision.collisionObjectB,
                });
                collision.collisionObjectB->OnCollision({
                    collision.hit,
                    collision.colliderB,
                    collision.collisionObjectA,
                });
            }

            for (auto collision: endedCollisions) {
                collision.collisionObjectA->OnCollisionEnded({
                    collision.hit,
                    collision.colliderA,
                    collision.collisionObjectB,
                });
                collision.collisionObjectB->OnCollisionEnded({
                    collision.hit,
                    collision.colliderB,
                    collision.collisionObjectA,
                });
            }

            this->collisionsEventVec = currentCollisions;
        }
};

#endif //CENGINE_COLLISION_H