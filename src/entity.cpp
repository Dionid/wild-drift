#include "entity.h"
#include "utils.h"

// # Paddle

Paddle::Paddle(
    Vector2 position,
    Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) : CharacterBody2D(position, size, velocity)
{
    this->speed = speed;
    this->maxVelocity = maxVelocity;
}

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

const uint64_t Paddle::_id = TypeIdGenerator::getInstance().getNextId();

// # Player
Player::Player(
    Vector2 position,
    Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) : Paddle(position, size, velocity, speed, maxVelocity)
{ 
};

const uint64_t Player::_id = TypeIdGenerator::getInstance().getNextId();

std::unique_ptr<Player> Player::NewPlayer(
    Vector2 position,
    Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) {
    auto player = std::make_unique<Player>(position, size, velocity, speed, maxVelocity);

    player->AddNode(
        std::make_unique<RectangleView>(
            size,
            BLUE
        )
    );

    player->AddNode(
        std::make_unique<Collider>(
            ColliderType::Solid,
            Shape::Rectangle(size),
            (Vector2){ 0.0f, 0.0f }
        )
    );

    return player;
}

// # Player Update function
void Player::Update(GameContext* ctx) {
    float deltaTime = DeltaTime();

    // # Calc velocity
    auto directionY = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    auto directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

    Vector2 newSpeed = Vector2Scale(
        Vector2Normalize({
            this->speed * directionX,
            this->speed * directionY,
        }),
        this->speed
    );

    this->velocity.y += newSpeed.y * deltaTime;
    this->velocity.x += newSpeed.x * deltaTime;

    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }

    // # Friction
    this->ApplyFriction();

    // # Field boundaries
    if (this->position.x + this->size.width/2 > ctx->worldWidth/2) {
        this->position.x = ctx->worldWidth/2 - this->size.width/2;
        this->velocity.x = 0;
    }

    // # World Boundaries
    this->ApplyWorldBoundaries(ctx->worldWidth, ctx->worldHeight);

    // # Velocity -> Position
    this->ApplyVelocityToPosition();
};

// # Ball
Ball::Ball(
    float radius,
    Vector2 position,
    Size size,
    Vector2 velocity = Vector2{},
    float maxVelocity = 10.0f
) : CharacterBody2D(position, size, velocity)
{
    this->radius = radius;
    this->maxVelocity = maxVelocity;
};

const uint64_t Ball::_id = TypeIdGenerator::getInstance().getNextId();

std::unique_ptr<Ball> Ball::NewBall(
    float ballRadius,
    float screenWidth,
    float screenHeight,
    float randomAngle
) {
    auto ball = std::make_unique<Ball>(
        ballRadius,
        (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
        (Size){ ballRadius*2, ballRadius*2 },
        (Vector2){ cos(randomAngle) * 5, sin(randomAngle) * 5 },
        7.0f
    );
    ball->AddNode(
        std::make_unique<CircleView>(
            ballRadius
        )
    );
    ball->AddNode(
        std::make_unique<Collider>(
            ColliderType::Solid,
            Shape::Circle(ballRadius),
            (Vector2){ 0.0f, 0.0f }
        )
    );

    return ball;
};

void Ball::OnCollisionStarted(Collision collision) {
    if (collision.other->TypeId() != Player::_id && collision.other->TypeId() != Enemy::_id) {
        return;
    }

    // TODO: Change this to over collision response (if you hit it from behind it reflects in wrong direction)
    this->velocity = Vector2Reflect(this->velocity, collision.hit.normal);
};

void Ball::OnCollision(Collision collision) {
    if (collision.otherCollider->type != ColliderType::Solid) {
        return;
    }

    // # Resolve penetration
    this->position = Vector2Add(
        this->position,
        Vector2Scale(collision.hit.normal, collision.hit.penetration)
    );

    // NOTE: This is other valid way of resolving collision
    // // # Reflect velocity
    // auto other = dynamic_cast<Paddle*>(collision.other);

    // if (other == nullptr) {
    //     return;
    // }

    // // # Resolve velocity
    // // ## Calculate velocity separation
    // float velocitySeparation = Vector2DotProduct(
    //     Vector2Subtract(this->velocity, other->velocity),
    //     collision.hit.normal
    // );

    // // ## Apply velocity separation
    // this->velocity = Vector2Add(
    //     this->velocity,
    //     Vector2Scale(collision.hit.normal, -2 * velocitySeparation)
    // );
};

void Ball::Update(GameContext* ctx) {
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

const uint64_t Enemy::_id = TypeIdGenerator::getInstance().getNextId();

Enemy::Enemy(
    Ball* ball,
    Vector2 position,
    Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) : Paddle(position, size, velocity, speed, maxVelocity)
{
    this->ball = ball;
};

std::unique_ptr<Enemy> Enemy::NewEnemy(
    Ball* ball,
    Vector2 position,
    Size size,
    Vector2 velocity = Vector2{},
    float speed = 5.0f,
    float maxVelocity = 10.0f
) {
    auto enemy = std::make_unique<Enemy>(
        ball,
        position,
        size,
        velocity,
        speed,
        maxVelocity
    );
    enemy->AddNode(
        std::make_unique<RectangleView>(
            size,
            RED
        )
    );
    enemy->AddNode(
        std::make_unique<Collider>(
            ColliderType::Solid,
            Shape::Rectangle(size),
            (Vector2){ 0.0f, 0.0f }
        )
    );

    return enemy;
};

void Enemy::Update(GameContext* ctx) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;
    float deltaTime = DeltaTime();

    float directionX = 0;
    float directionY = 0;

    // # AI
    if (this->ball != nullptr) {
        if (this->position.y > this->ball->position.y + this->ball->radius + 50) {
            directionY = -1;
        } else if (this->position.y < this->ball->position.y - this->ball->radius - 50) {
            directionY = 1;
        }

        if (this->ball->position.x < worldWidth/2) {
            directionX = 1;
        } else {
            directionX = -1;
        }

        // # Ball is behind
        if (this->ball->position.x + this->ball->radius > this->position.x - this->size.width/2 + this->size.width/10) {
            directionX = 1;
        }
    }

    // # Calc velocity
    Vector2 newSpeed = Vector2Scale(
        Vector2Normalize({
            this->speed * directionX,
            this->speed * directionY,
        }),
        this->speed
    );

    this->velocity.y += newSpeed.y * deltaTime;
    this->velocity.x += newSpeed.x * deltaTime;

    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }

    // # Friction
    this->ApplyFriction();

    // # Field boundaries
    if (this->position.x - this->size.width/2 < worldWidth/2) {
        this->position.x = worldWidth/2 + this->size.width/2;
        this->velocity.x = 0;
    }

    // # World Boundaries
    this->ApplyWorldBoundaries(worldWidth, worldHeight);

    // # Velocity -> Position
    this->ApplyVelocityToPosition();
}

// # Goal

const uint64_t Goal::_id = TypeIdGenerator::getInstance().getNextId();

Goal::Goal(
    bool isLeft,
    Vector2 position,
    Size size
) : CollisionObject2D(position)
{
    this->isLeft = isLeft;
};

std::unique_ptr<Goal> Goal::NewGoal(
    bool isLeft,
    Vector2 position,
    Size size
) {
    auto goal = std::make_unique<Goal>(isLeft, position, size);

    goal->AddNode(
        std::make_unique<RectangleView>(
            size,
            WHITE,
            0.3f
        )
    );

    goal->AddNode(
        std::make_unique<Collider>(
            ColliderType::Sensor,
            Shape::Rectangle(size)
        )
    );

    return goal;
}

// # Score Manager

ScoreManager::ScoreManager(): Node() {
    this->playerScore = 0;
    this->enemyScore = 0;
}

void ScoreManager::PlayerScored() {
    this->playerScore++;
}

void ScoreManager::EnemyScored() {
    this->enemyScore++;
}

void ScoreManager::Update(GameContext* ctx) {
    for (const auto& collision: ctx->collisionEngine->startedCollisions) {
        bool predicate = (
            collision.collisionObjectA->TypeId() == Ball::_id &&
            collision.collisionObjectB->TypeId() == Goal::_id
        ) || (
            collision.collisionObjectA->TypeId() == Goal::_id &&
            collision.collisionObjectB->TypeId() == Ball::_id
        );

        if (
            !predicate
        ) {
            return;
        }

        Ball* ball;
        Goal* goal;

        if (collision.collisionObjectA->TypeId() == Ball::_id) {
            ball = static_cast<Ball*>(collision.collisionObjectA);
            goal = static_cast<Goal*>(collision.collisionObjectB);
        } else {
            ball = static_cast<Ball*>(collision.collisionObjectB);
            goal = static_cast<Goal*>(collision.collisionObjectA);
        }

        if (goal->isLeft) {
            this->EnemyScored();
        } else {
            this->PlayerScored();
        }
    }
}

void ScoreManager::Render(GameContext* ctx) {
    DrawText(
        std::to_string(this->playerScore).c_str(),
        40,
        20,
        20,
        GREEN
    );

    DrawText(
        std::to_string(this->enemyScore).c_str(),
        ctx->worldWidth - 40,
        20,
        20,
        RED
    );
}