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
};

// # Player
class Player: public Paddle {
    public:
        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Player::_tid;
        }

        Player(
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update(cen::GameContext* ctx) override;
        void Init(cen::GameContext* ctx) override;
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

        void Init(cen::GameContext* ctx) override;
        void Update(cen::GameContext* ctx) override;
        void OnCollision(cen::Collision c) override;
        void OnCollisionStarted(cen::Collision c) override;
};

// # Enemy

class Enemy: public Paddle {
    public:
        cen::node_id_t ballId;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Enemy::_tid;
        }

        Enemy(
            cen::node_id_t ballId,
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Init(cen::GameContext* ctx) override;
        void Update(cen::GameContext* ctx) override;
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

        void Init(cen::GameContext* ctx) override;
};

struct StartEvent: public cen::Event {
    static const std::string type;
    StartEvent(): cen::Event(StartEvent::type) {}
};

struct RestartEvent: public cen::Event {
    static const std::string type;
    RestartEvent(): cen::Event(RestartEvent::type) {}
};

#endif // CSP_ENTITY_H_