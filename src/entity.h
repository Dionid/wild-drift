#ifndef CSP_ENTITY_H_
#define CSP_ENTITY_H_

#include "cengine.h"

// # Paddle
class Paddle: public CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

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
        Player(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        static std::shared_ptr<Player> NewPlayer(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update(GameContext* ctx) override;
};

// # Enemy
class Enemy: public Paddle {
    public:
        Enemy(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update(GameContext* ctx) override;

        static std::shared_ptr<Enemy> NewEnemy(
            Vector2 position,
            Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );
};

// # Ball
class Ball: public CharacterBody2D {
    public:
        float radius;
        float maxVelocity;

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

        static std::shared_ptr<Ball> NewBall(
            float ballRadius,
            float screenWidth,
            float screenHeight,
            float randomAngle
        );
};

#endif // CSP_ENTITY_H_