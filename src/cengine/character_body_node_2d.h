#ifndef CENGINE_CHARACTER_BODY_NODE_H
#define CENGINE_CHARACTER_BODY_NODE_H

#include "core.h"
#include "scene.h"
#include "node_2d.h"
#include "collision.h"

namespace cen {

enum class MotionMode {
    Floating,
    Grounded,
};

class CharacterBody2D: public CollisionObject2D {
    public:
        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return CharacterBody2D::_tid;
        }

        cen::Size size;
        Vector2 velocity;
        MotionMode motionMode;
        float skinWidth;
        Vector2 up;

        CharacterBody2D(
            Vector2 position,
            cen::Size size,
            Vector2 velocity = Vector2{},
            MotionMode motionMode = MotionMode::Floating,
            Vector2 up = cen::Vector2Up,
            float skinWidth = 0.1f,
            int zOrder = 0,
            uint16_t id = 0,
            Node* parent = nullptr
        ) : CollisionObject2D(position, zOrder, id, parent) {
            this->size = size;
            this->velocity = velocity;
        }

        void ApplyVelocityToPosition() {
            this->position.x += this->velocity.x;
            this->position.y += this->velocity.y;
        }

        void MoveAndSlide(
            cen::GameContext* ctx
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

            for (const auto& node: this->children) {
                auto collider = node->GetFirstByType<Collider>();

                if (collider == nullptr) {
                    continue;
                }

                if (collider->type == ColliderType::Sensor) {
                    continue;
                }

                for (const auto& otherNode: ctx->scene->nodeStorage->rootNodes) {
                    if (this == otherNode.get()) {
                        continue;
                    }

                    auto otherCollisionObject = otherNode->GetFirstByType<CollisionObject2D>();

                    if (otherCollisionObject == nullptr) {
                        continue;
                    }

                    for (const auto& otherN: otherCollisionObject->children) {
                        auto otherCollider = dynamic_cast<Collider*>(otherN.get());

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
            cen::GameContext* ctx
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

            for (const auto& node: this->children) {
                auto collider = dynamic_cast<Collider*>(node.get());

                if (collider == nullptr) {
                    continue;
                }

                if (collider->type == ColliderType::Sensor) {
                    continue;
                }

                for (const auto& otherNode: ctx->scene->nodeStorage->rootNodes) {
                    if (this == otherNode.get()) {
                        continue;
                    }

                    auto otherCollisionObject = otherNode->GetFirstByType<CollisionObject2D>();
                    if (otherCollisionObject == nullptr) {
                        continue;
                    }

                    for (const auto& otherN: otherNode->children) {
                        auto otherCollider = dynamic_cast<Collider*>(otherN.get());

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

}

#endif // CENGINE_CHARACTER_BODY_NODE_H