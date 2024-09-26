#ifndef CSP_ENTITY_H_
#define CSP_ENTITY_H_

#include "cengine/cengine.h"
#include "audio.h"

// # Paddle
class Paddle: public cen::CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Paddle::_tid;
        }

        Paddle(
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void ApplyFriction();
        void ApplyWorldBoundaries(float worldWidth, float worldHeight);

        void Move(int directionX, int directionY);
        virtual void ApplyFieldBoundaries() = 0;
};

// # Player
class Player: public Paddle {
    public:
        static const uint64_t _tid;
        cen::player_id_t playerId;
        bool leftSide;

        int directionY;
        int directionX;

        cen::type_id_t TypeId() const override {
            return Player::_tid;
        }

        Player(
            bool leftSide,
            cen::player_id_t playerId,
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update() override;
        void FixedUpdate() override;
        void Init() override;
        void ApplyFieldBoundaries() override;

        void Reset();
};

// # Ball
class Ball: public cen::CharacterBody2D {
    public:
        float radius;
        float maxVelocity;
        SpcAudio* gameAudio;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Ball::_tid;
        }

        Ball(
            SpcAudio* gameAudio,
            float radius,
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float maxVelocity
        );

        void Init() override;
        void FixedUpdate() override;
        void OnCollision(cen::Collision c) override;
        void OnCollisionStarted(cen::Collision c) override;
};

// # AiOpponent
class AiOpponent: public Player {
    public:
        cen::node_id_t ballId;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return AiOpponent::_tid;
        }

        AiOpponent(
            cen::node_id_t ballId,
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update() override;
};

// # Goal

class Goal: public cen::CollisionObject2D {
    public:
        static const uint64_t _tid;
        bool isLeft;
        cen::Size size;
        Vector2 position;

        cen::type_id_t TypeId() const override {
            return Goal::_tid;
        }

        Goal(
            bool isLeft,
            Vector2 position,
            cen::Size size
        );

        void Init() override;
};

#endif // CSP_ENTITY_H_