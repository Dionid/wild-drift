#include <raylib.h>
#include "core.h"
#include "node-2d.h"
#include "collision.h"

#ifndef CENGINE_CHARACTER_BODY_NODE_H
#define CENGINE_CHARACTER_BODY_NODE_H

enum class MotionMode {
    Floating,
    Grounded,
};

class CharacterBody2D: public CollisionObject2D {
    public:
        Size size;
        Vector2 velocity;
        MotionMode motionMode;
        float skinWidth;
        Vector2 up;

        CharacterBody2D(
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            MotionMode motionMode = MotionMode::Floating,
            Vector2 up = Vector2Up,
            float skinWidth = 0.1f,
            std::shared_ptr<Node> parent = nullptr
        ) : CollisionObject2D(position, parent) {
            this->size = size;
            this->velocity = velocity;
        }

        void ApplyVelocityToPosition() {
            this->position.x += this->velocity.x;
            this->position.y += this->velocity.y;
        }

        void MoveAndSlide(
            GameContext* ctx
        ) {
            if (this->velocity.x == 0 && this->velocity.y == 0) {
                return;
            }

            auto newPosition = Vector2Add(
                Vector2Add(
                    this->GlobalPosition(),
                    (Vector2){
                        this->skinWidth,
                        this->skinWidth
                    }
                ),
                this->velocity
            );

            for (auto node: this->nodes) {
                auto collider = std::dynamic_pointer_cast<Collider>(node);

                if (collider == nullptr) {
                    continue;
                }

                if (collider->type == ColliderType::Sensor) {
                    continue;
                }

                for (auto otherNode: ctx->nodes) {
                    if (this == otherNode.get()) {
                        continue;
                    }

                    // TODO: refactor
                    auto otherCollisionObject = std::dynamic_pointer_cast<CollisionObject2D>(otherNode);

                    if (otherCollisionObject == nullptr) {
                        continue;
                    }

                    // TODO: recursive nodes
                    for (auto otherN: otherNode->nodes) {
                        auto otherCollider = std::dynamic_pointer_cast<Collider>(otherN);

                        if (otherCollider == nullptr) {
                            continue;
                        }

                        if (otherCollider->type == ColliderType::Sensor) {
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
                                            otherCollider->GlobalPosition(),
                                            otherCollider->shape.circle.radius,
                                            newPosition,
                                            collider->shape.rect.size
                                        );
                                        break;
                                }
                                break;
                            case Shape::Type::CIRCLE:
                                switch (otherCollider->shape.type) {
                                    case Shape::Type::RECTANGLE:
                                        collision = CircleRectangleCollision(
                                            newPosition,
                                            collider->shape.circle.radius,
                                            otherCollider->GlobalPosition(),
                                            otherCollider->shape.rect.size
                                        );
                                        break;
                                    case Shape::Type::CIRCLE:
                                        break;
                                }
                                break;
                        }

                        if (collision.penetration > 0) {
                            auto d = Vector2DotProduct(this->velocity, collision.normal);
                            if (d < 0) {
                                this->velocity.x += collision.normal.x * collision.penetration;
                                this->velocity.y += collision.normal.y * collision.penetration;
                            } else {
                                this->velocity.x -= collision.normal.x * collision.penetration;
                                this->velocity.y -= collision.normal.y * collision.penetration;
                            }
                        }
                    }
                }
            }

            this->ApplyVelocityToPosition();
        }

        std::vector<Collision> MoveAndCollide(
            GameContext* ctx
        ) {
            std::vector<Collision> collisions;

            auto newPosition = Vector2Add(
                Vector2Add(
                    this->GlobalPosition(),
                    (Vector2){
                        this->skinWidth,
                        this->skinWidth
                    }
                ),
                this->velocity
            );

            for (auto node: this->nodes) {
                auto collider = std::dynamic_pointer_cast<Collider>(node);

                if (collider == nullptr) {
                    continue;
                }

                if (collider->type == ColliderType::Sensor) {
                    continue;
                }

                for (auto otherNode: ctx->nodes) {
                    if (this == otherNode.get()) {
                        continue;
                    }

                    // TODO: refactor
                    auto otherCollisionObject = std::dynamic_pointer_cast<CollisionObject2D>(otherNode);

                    if (otherCollisionObject == nullptr) {
                        continue;
                    }

                    // TODO: recursive nodes
                    for (auto otherN: otherNode->nodes) {
                        auto otherCollider = std::dynamic_pointer_cast<Collider>(otherN);

                        if (otherCollider == nullptr) {
                            continue;
                        }

                        if (otherCollider->type == ColliderType::Sensor) {
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
                                            otherCollider->GlobalPosition(),
                                            otherCollider->shape.circle.radius,
                                            newPosition,
                                            collider->shape.rect.size
                                        );
                                        break;
                                }
                                break;
                            case Shape::Type::CIRCLE:
                                switch (otherCollider->shape.type) {
                                    case Shape::Type::RECTANGLE:
                                        collision = CircleRectangleCollision(
                                            newPosition,
                                            collider->shape.circle.radius,
                                            otherCollider->GlobalPosition(),
                                            otherCollider->shape.rect.size
                                        );
                                        break;
                                    case Shape::Type::CIRCLE:
                                        break;
                                }
                                break;
                        }

                        if (collision.penetration > 0) {
                            collisions.push_back(Collision{
                                collision,
                                collider,
                                otherCollisionObject
                            });
                        }
                    }
                }
            }

            return collisions;
        }
};

#endif // CENGINE_CHARACTER_BODY_NODE_H