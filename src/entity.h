#ifndef CSP_ENTITY_H_
#define CSP_ENTITY_H_

#include "cengine/cengine.h"

// # Paddle
class Paddle: public CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

        static const uint64_t _tid;

        type_id_t TypeId() const override {
            return Paddle::_tid;
        }

        Paddle(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void ApplyFriction();
        void ApplyWorldBoundaries(float worldWidth, float worldHeight);
};

// # Player
class Player: public Paddle {
    public:
        static const uint64_t _tid;

        type_id_t TypeId() const override {
            return Player::_tid;
        }

        Player(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        static std::unique_ptr<Player> NewPlayer(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update(GameContext* ctx) override;
        void Init(GameContext* ctx) override;
};

// # Ball
class Ball: public CharacterBody2D {
    public:
        float radius;
        float maxVelocity;

        static const uint64_t _tid;

        type_id_t TypeId() const override {
            return Ball::_tid;
        }

        Ball(
            float radius,
            Vector2 position,
            Size size,
            Vector2 velocity,
            float maxVelocity
        );

        void Init(GameContext* ctx) override;
        void Update(GameContext* ctx) override;
        void OnCollision(Collision c) override;
        void OnCollisionStarted(Collision c) override;

        static std::unique_ptr<Ball> NewBall(
            float ballRadius,
            float screenWidth,
            float screenHeight,
            float randomAngle
        );
};

// # Enemy

class Enemy: public Paddle {
    public:
        node_id_t ballId;

        static const uint64_t _tid;

        type_id_t TypeId() const override {
            return Enemy::_tid;
        }

        Enemy(
            node_id_t ballId,
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Init(GameContext* ctx) override;
        void Update(GameContext* ctx) override;

        static std::unique_ptr<Enemy> NewEnemy(
            node_id_t ballId,
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );
};

// # Goal

class Goal: public CollisionObject2D {
    public:
        static const uint64_t _tid;
        bool isLeft;
        Size size;
        Vector2 position;

        type_id_t TypeId() const override {
            return Goal::_tid;
        }

        Goal(
            bool isLeft,
            Vector2 position,
            Size size
        );

        void Init(GameContext* ctx) override;

        static std::unique_ptr<Goal> NewGoal(
            bool isLeft,
            Vector2 position,
            Size size
        );
};

#endif // CSP_ENTITY_H_