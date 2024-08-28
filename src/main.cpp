#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include "core.h"
#include "engine.h"
#include "view.h"
#include "node-2d.h"
#include "collision.h"
#include "debug.h"

using namespace std;

const float FPS = 60.0f;
const float secondsPerFrame = 1.0f / FPS;

float DeltaTime() {
    return GetFrameTime() / secondsPerFrame;
}

void CharacterApplyFriction(CharacterBody2D *c) {
    c->velocity.y *= .80f;
    c->velocity.x *= .80f;

    if (c->velocity.x < 0.01 && c->velocity.x > -0.01) {
        c->velocity.x = 0;
    }

    if (c->velocity.y < 0.01 && c->velocity.y > -0.01) {
        c->velocity.y = 0;
    }
}

void CharacterApplyWorldBoundaries(CharacterBody2D *c, float worldWidth, float worldHeight) {
    if (c->position.x - c->size.width/2 < 0) {
        c->position.x = c->size.width/2;
        c->velocity.x = 0;
    } else if (c->position.x + c->size.width/2 > worldWidth) {
        c->position.x = worldWidth - c->size.width/2;
        c->velocity.x = 0;
    }

    if (c->position.y - c->size.height/2 < 0) {
        c->position.y = c->size.height/2;
        c->velocity.y = 0;
    } else if (c->position.y + c->size.height/2 > worldHeight) {
        c->position.y = worldHeight - c->size.height/2;
        c->velocity.y = 0;
    }
}

// # Paddle

class Paddle: public CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

        Paddle(
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
};

// # Player
class Player: public Paddle {
    public:
        shared_ptr<RectangleView> view;
        shared_ptr<Collider> collider;

        Player(
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            float speed = 5.0f,
            float maxVelocity = 10.0f
        ) : Paddle(position, size, velocity, speed, maxVelocity)
        { 
        }

        static shared_ptr<Player> NewPlayer(
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            float speed = 5.0f,
            float maxVelocity = 10.0f
        ) {
            auto player = make_shared<Player>(position, size, velocity, speed, maxVelocity);

            player->view = player->AddNode(
                make_shared<RectangleView>(
                    size,
                    BLUE
                )
            );

            player->collider = player->AddNode(
                make_shared<Collider>(
                    ColliderType::Solid,
                    Shape::Rectangle(size),
                    (Vector2){ 0.0f, 0.0f }
                )
            );

            return player;
        }

        void Update(GameContext* ctx) override;
};

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
    CharacterApplyFriction(this);

    // # Field boundaries
    if (this->position.x + this->size.width/2 > ctx->worldWidth/2) {
        this->position.x = ctx->worldWidth/2 - this->size.width/2;
        this->velocity.x = 0;
    }

    // # World Boundaries
    CharacterApplyWorldBoundaries(this, ctx->worldWidth, ctx->worldHeight);

    // # Velocity -> Position
    this->ApplyVelocityToPosition();
}

// # Ball
class Ball: public CharacterBody2D {
    public:
        float radius;
        float maxVelocity;
        Ball(
            float radius,
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            float maxVelocity = 10.0f
        ) : CharacterBody2D(position, size, velocity)
        {
            this->radius = radius;
            this->maxVelocity = maxVelocity;
        }

        void Update(GameContext* ctx) override;
        void OnCollision(Collision c) override;
        void OnCollisionStarted(Collision c) override;

        static std::shared_ptr<Ball> NewBall(
            float ballRadius,
            float screenWidth,
            float screenHeight,
            float randomAngle
        ) {
            auto ball = make_shared<Ball>(
                ballRadius,
                (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
                (Size){ ballRadius*2, ballRadius*2 },
                (Vector2){ cos(randomAngle) * 5, sin(randomAngle) * 5 },
                7.0f
            );
            ball->AddNode(
                make_shared<CircleView>(
                    ballRadius
                )
            );
            ball->AddNode(
                make_shared<Collider>(
                    ColliderType::Solid,
                    Shape::Circle(ballRadius),
                    (Vector2){ 0.0f, 0.0f }
                )
            );

            return ball;
        }
};

void Ball::OnCollisionStarted(Collision collision) {
    this->velocity = Vector2Reflect(this->velocity, collision.hit.normal);
}

void Ball::OnCollision(Collision collision) {
    // # Resolve penetration
    this->position = Vector2Add(
        this->position,
        Vector2Scale(collision.hit.normal, collision.hit.penetration)
    );

    // # This is other valid way of resolving collision
    // // # Reflect velocity
    // auto other = dynamic_pointer_cast<Paddle>(collision.other);

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
}

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
}

// # Enemy
class Enemy: public Paddle {
    public:
        Enemy(
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            float speed = 5.0f,
            float maxVelocity = 10.0f
        ) : Paddle(position, size, velocity, speed, maxVelocity)
        {
        }

        void Update(GameContext* ctx) override;

        static std::shared_ptr<Enemy> NewEnemy(
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            float speed = 5.0f,
            float maxVelocity = 10.0f
        ) {
            auto enemy = make_shared<Enemy>(
                position,
                size,
                velocity,
                speed,
                maxVelocity
            );
            enemy->AddNode(
                make_shared<RectangleView>(
                    size,
                    RED
                )
            );
            enemy->AddNode(
                make_shared<Collider>(
                    ColliderType::Solid,
                    Shape::Rectangle(size),
                    (Vector2){ 0.0f, 0.0f }
                )
            );

            return enemy;
        }
};

void Enemy::Update(GameContext* ctx) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;
    float deltaTime = DeltaTime();

    float directionX = 0;
    float directionY = 0;

    // # AI
    for (auto node: ctx->nodes) {
        if (node.get() == this) {
            continue;
        }

        auto ball = dynamic_pointer_cast<Ball>(node);
        if (ball == nullptr) {
            continue;
        }

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
    CharacterApplyFriction(this);

    // # Field boundaries
    if (this->position.x - this->size.width/2 < worldWidth/2) {
        this->position.x = worldWidth/2 + this->size.width/2;
        this->velocity.x = 0;
    }

    // # World Boundaries
    CharacterApplyWorldBoundaries(this, worldWidth, worldHeight);

    // # Velocity -> Position
    this->ApplyVelocityToPosition();
}

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    DisableCursor();
    SetTargetFPS(FPS);

    Debugger debugger;

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // # Player
    const float sixthScreen = screenWidth/6.0f;

    auto player = Player::NewPlayer(
        (Vector2){ sixthScreen, screenHeight/2.0f },
        (Size){ 40.0f, 120.0f },
        (Vector2){ 0.0f, 0.0f },
        1.0f,
        10.0f
    );

    auto enemy = Enemy::NewEnemy(
        (Vector2){ screenWidth - sixthScreen, screenHeight/2.0f },
        (Size){ 40.0f, 120.0f },
        (Vector2){ 0.0f, 0.0f },
        1.0f,
        10.0f
    );

    float ballRadius = 15.0f;
    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * PI * 2;
    auto ball = Ball::NewBall(
        ballRadius,
        screenWidth,
        screenHeight,
        7.0f
    );

    // # Field
    auto line = make_shared<LineView>(
        (Vector2){ screenWidth/2.0f, 80 },
        screenHeight - 160,
        WHITE,
        0.5f
    );

    auto circle = make_shared<CircleView>(
        80,
        (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
        WHITE,
        0.5f,
        false
    );

    // # Game Context
    GameContext ctx = {
        { ball, player, enemy, line, circle },
        screenWidth,
        screenHeight
    };

    CollisionEngine collisionEngine;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // # Initial
        for (auto i = 0; i < ctx.nodes.size(); i++) {
            auto node = ctx.nodes[i];
            node->TraverseNodeUpdate(&ctx);
        }

        // # Collision
        collisionEngine.NarrowCollisionCheckNaive(&ctx);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (auto node: ctx.nodes) {
                node->TraverseNodeRender(&ctx);
            }
            debugger.Render(&ctx);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}