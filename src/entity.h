#ifndef CSP_ENTITY_H_
#define CSP_ENTITY_H_

#include "cengine/cengine.h"

// # Paddle
class Paddle: public CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

        static const uint64_t _id;

        uint64_t TypeId() const override {
            return Paddle::_id;
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
        static const uint64_t _id;

        uint64_t TypeId() const override {
            return Player::_id;
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
};

// # Ball
class Ball: public CharacterBody2D {
    public:
        float radius;
        float maxVelocity;

        static const uint64_t _id;

        uint64_t TypeId() const override {
            return Ball::_id;
        }

        Ball(
            float radius,
            Vector2 position,
            Size size,
            Vector2 velocity,
            float maxVelocity
        );

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
        Ball* ball;

        static const uint64_t _id;

        uint64_t TypeId() const override {
            return Enemy::_id;
        }

        Enemy(
            Ball* ball,
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update(GameContext* ctx) override;

        static std::unique_ptr<Enemy> NewEnemy(
            Ball* ball,
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );
};

// # Goal

class Goal: public Node2D {
    public:
        static const uint64_t _id;

        uint64_t TypeId() const override {
            return Goal::_id;
        }

        Goal(
            Vector2 position,
            Size size
        );

        static std::unique_ptr<Goal> NewGoal(
            Vector2 position,
            Size size
        );
};

#endif // CSP_ENTITY_H_