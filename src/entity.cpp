#include "entity.h"
#include "utils.h"
#include "audio.h"

// # Paddle

Paddle::Paddle(
    Vector2 position,
    cen::Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) : CharacterBody2D(position, size, velocity)
{
    this->speed = speed;
    this->maxVelocity = maxVelocity;
}

const uint64_t Paddle::_tid = cen::TypeIdGenerator::getInstance().getNextId();

// TODO: Move constants to props
void Paddle::ApplyFriction() {
    this->velocity.y *= .80f;
    this->velocity.x *= .80f;

    if (this->velocity.x < 0.01 && this->velocity.x > -0.01) {
        this->velocity.x = 0;
    }

    if (this->velocity.y < 0.01 && this->velocity.y > -0.01) {
        this->velocity.y = 0;
    }
}

// TODO: Move constants to props
void Paddle::ApplyWorldBoundaries(float worldWidth, float worldHeight) {
    if (this->position.x - this->size.width/2 < 0) {
        this->position.x = this->size.width/2;
        this->velocity.x = 0;
    } else if (this->position.x + this->size.width/2 > worldWidth) {
        this->position.x = worldWidth - this->size.width/2;
        this->velocity.x = 0;
    }

    if (this->position.y - this->size.height/2 < 0) {
        this->position.y = this->size.height/2;
        this->velocity.y = 0;
    } else if (this->position.y + this->size.height/2 > worldHeight) {
        this->position.y = worldHeight - this->size.height/2;
        this->velocity.y = 0;
    }
};

void Paddle::Move(
    cen::GameContext* ctx,
    int directionX,
    int directionY
) {
    Vector2 newSpeed = Vector2Scale(
        Vector2Normalize({
            this->speed * directionX,
            this->speed * directionY,
        }),
        this->speed
    );

    this->velocity.y += newSpeed.y;
    this->velocity.x += newSpeed.x;

    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }

    // # Friction
    this->ApplyFriction();

    // # Field boundaries
    this->ApplyFieldBoundaries(ctx);

    // # World Boundaries
    this->ApplyWorldBoundaries(ctx->worldWidth, ctx->worldHeight);

    // # Velocity -> Position
    this->ApplyVelocityToPosition();
}

// # Player
Player::Player(
    Vector2 position,
    cen::Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) : Paddle(position, size, velocity, speed, maxVelocity)
{ 
};

const uint64_t Player::_tid = cen::TypeIdGenerator::getInstance().getNextId();

void Player::Init(cen::GameContext* ctx) {
    this->AddNode(
        std::make_unique<cen::RectangleView>(
            this->size,
            WHITE
        )
    );

    this->AddNode(
        std::make_unique<cen::Collider>(
            cen::ColliderType::Solid,
            cen::Shape::Rectangle(this->size),
            (Vector2){ 0.0f, 0.0f }
        )
    );
};

void Player::ApplyFieldBoundaries(cen::GameContext* ctx) {
    if (this->position.x + this->size.width/2 > ctx->worldWidth/2) {
        this->position.x = ctx->worldWidth/2 - this->size.width/2;
        this->velocity.x = 0;
    }
}

// # Player Update function
void Player::FixedUpdate(cen::GameContext* ctx) {
    // # Calc velocity
    auto directionY = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    auto directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

    this->Move(
        ctx,
        directionX,
        directionY
    );
};

// # Ball
Ball::Ball(
    SpcAudio* gameAudio,
    float radius,
    Vector2 position,
    cen::Size size,
    Vector2 velocity = Vector2{},
    float maxVelocity = 10.0f
) : CharacterBody2D(position, size, velocity)
{
    this->gameAudio = gameAudio;
    this->radius = radius;
    this->maxVelocity = maxVelocity;
};

const uint64_t Ball::_tid = cen::TypeIdGenerator::getInstance().getNextId();

void Ball::Init(cen::GameContext* ctx) {
    this->AddNode(
        std::make_unique<cen::CircleView>(
            this->radius
        )
    );

    this->AddNode(
        std::make_unique<cen::Collider>(
            cen::ColliderType::Solid,
            cen::Shape::Circle(this->radius),
            (Vector2){ 0.0f, 0.0f }
        )
    );
}

void Ball::OnCollisionStarted(cen::Collision collision) {
    if (collision.other->TypeId() != Player::_tid && collision.other->TypeId() != Enemy::_tid) {
        return;
    }

    SetSoundPitch(this->gameAudio->hit, GetRandomValue(80, 120) / 100.0f);
    PlaySound(this->gameAudio->hit);

    auto other = dynamic_cast<Paddle*>(collision.other);

    if (other == nullptr) {
        return;
    }

    // # Resolve velocity
    // ## Calculate velocity separation
    float velocitySeparation = Vector2DotProduct(
        Vector2Subtract(this->velocity, other->velocity),
        collision.hit.normal
    );

    // ## Apply velocity separation
    this->velocity = Vector2Add(
        this->velocity,
        Vector2Scale(collision.hit.normal, -2 * velocitySeparation)
    );
};

void Ball::OnCollision(cen::Collision collision) {
    if (collision.otherCollider->type != cen::ColliderType::Solid) {
        return;
    }

    // # Resolve penetration
    this->position = Vector2Add(
        this->position,
        Vector2Scale(collision.hit.normal, collision.hit.penetration)
    );
};

void Ball::FixedUpdate(cen::GameContext* ctx) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;

    // # World Boundaries
    if (this->position.x - this->radius < 0) {
        this->position.x = this->radius;
        this->velocity = Vector2Reflect(this->velocity, (Vector2){1, 0});
    } else if (this->position.x + this->radius > worldWidth) {
        this->position.x = worldWidth - this->radius;
        this->velocity = Vector2Reflect(this->velocity, (Vector2){-1, 0});
    }

    if (this->position.y - this->radius < 0) {
        this->position.y = this->radius;
        this->velocity = Vector2Reflect(this->velocity, (Vector2){0, -1});
    } else if (this->position.y + this->radius > worldHeight) {
        this->position.y = worldHeight - this->radius;
        this->velocity = Vector2Reflect(this->velocity, (Vector2){0, 1});
    }

    // # Limit and normalize velocity
    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }

    this->ApplyVelocityToPosition();
};

// # Enemy

const uint64_t Enemy::_tid = cen::TypeIdGenerator::getInstance().getNextId();

Enemy::Enemy(
    cen::node_id_t ballId,
    Vector2 position,
    cen::Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) : Paddle(position, size, velocity, speed, maxVelocity)
{
    this->ballId = ballId;
};

void Enemy::Init(cen::GameContext* ctx) {
    this->AddNode(
        std::make_unique<cen::RectangleView>(
            this->size,
            WHITE
        )
    );
    this->AddNode(
        std::make_unique<cen::Collider>(
            cen::ColliderType::Solid,
            cen::Shape::Rectangle(this->size),
            (Vector2){ 0.0f, 0.0f }
        )
    );
}

void Enemy::ApplyFieldBoundaries(cen::GameContext* ctx) {
    if (this->position.x - this->size.width/2 < ctx->worldWidth/2) {
        this->position.x = ctx->worldWidth/2 + this->size.width/2;
        this->velocity.x = 0;
    }
}

void Enemy::FixedUpdate(cen::GameContext* ctx) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;

    float directionX = 0;
    float directionY = 0;

    auto ball = ctx->scene->node_storage->GetById<Ball>(this->ballId);

    // # AI
    if (ball != nullptr) {
        if (this->position.y > ball->position.y + ball->radius + 50) {
            directionY = -1;
        } else if (this->position.y < ball->position.y - ball->radius - 50) {
            directionY = 1;
        }

        if (ball->position.x < worldWidth/2) {
            directionX = 1;
        } else {
            directionX = -1;
        }

        // # Ball is behind
        if (ball->position.x + ball->radius > this->position.x - this->size.width/2 + this->size.width/10) {
            directionX = 1;
        }
    }

    this->Move(
        ctx,
        directionX,
        directionY
    );
}

// # Goal

const uint64_t Goal::_tid = cen::TypeIdGenerator::getInstance().getNextId();

Goal::Goal(
    bool isLeft,
    Vector2 position,
    cen::Size size
) : CollisionObject2D(position)
{
    this->isLeft = isLeft;
    this->size = size;
    this->position = position;
};

void Goal::Init(cen::GameContext* ctx) {
    this->AddNode(
        std::make_unique<cen::RectangleView>(
            this->size,
            WHITE,
            0.3f
        )
    );

    this->AddNode(
        std::make_unique<cen::Collider>(
            cen::ColliderType::Sensor,
            cen::Shape::Rectangle(this->size)
        )
    );
}

// # Events

const std::string StartEvent::type = "StartEvent";
const std::string RestartEvent::type = "RestartEvent";