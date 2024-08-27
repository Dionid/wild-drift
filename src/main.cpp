#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include "core.h"
#include "engine.h"
#include "view.h"
#include "nodes.h"
#include "collision.h"
#include "debug.h"

using namespace std;

void CharacterApplyFriction(CharacterBody2D *c) {
    c->velocity.y *= .80f;
    c->velocity.x *= .80f;

    if (c->velocity.x < 0.001 && c->velocity.x > -0.001) {
        c->velocity.x = 0;
    }

    if (c->velocity.y < 0.001 && c->velocity.y > -0.001) {
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

// # Player
class Player: public CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

        Player(
            Vector2 position,
            Size size,
            Vector2 velocity = Vector2{},
            float speed = 5.0f,
            float maxVelocity = 10.0f
        ) : CharacterBody2D(position, size, velocity, MotionMode::Floating, Vector2Up)
        {
            this->speed = speed;
            this->maxVelocity = maxVelocity;   
        }

        void Update(GameContext* ctx, GameObject* thisGO) override;
        void Render(GameContext* ctx, GameObject* thisGO) override;
};

// # Player Update function
void Player::Update(GameContext* ctx, GameObject* thisGO) {
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
    // this->ApplyVelocityToPosition();
    this->MoveAndSlide(thisGO, ctx);
}

void Player::Render(GameContext* ctx, GameObject* thisGO) {
    // Rectangle playerRect = { this->position.x - this->size.width/2, this->position.y - this->size.height/2, this->size.width, this->size.height };
    // DrawRectangleRec(playerRect, BLUE);
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

        void Update(GameContext* ctx, GameObject* thisGO) override;
        void Render(GameContext* ctx, GameObject* thisGO) override;
};

void Ball::Update(GameContext* ctx, GameObject* thisGO) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;
    auto gos = ctx->gos;

    // # Limit and normalize velocity
    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }

    // // # Velocity -> Position
    // this->ApplyVelocityToPosition();

    this->MoveAndSlide(thisGO, ctx);

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
}

void Ball::Render(GameContext* ctx, GameObject* thisGO) {
    // DrawCircleV(this->position, this->radius, WHITE);
}

// # Ball
class Enemy: public CharacterBody2D {
    public:
        float maxVelocity;
        float speed;

        Enemy(
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

        void Update(GameContext* ctx, GameObject* thisGO) override;
        void Render(GameContext* ctx, GameObject* thisGO) override;
};

void Enemy::Update(GameContext* ctx, GameObject* thisGO) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;
    auto gos = ctx->gos;
    float deltaTime = DeltaTime();

    float directionX = 0;
    float directionY = 0;

    // # AI
    for (auto go: gos) {
        if (go == thisGO) {
            continue;
        }

        auto ball = dynamic_pointer_cast<Ball>(go->rootNode);
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
    this->MoveAndSlide(thisGO, ctx);
}

void Enemy::Render(GameContext* ctx, GameObject* thisGO) {
    // Rectangle enemyRect = { this->position.x - this->size.width/2, this->position.y - this->size.height/2, this->size.width, this->size.height };
    // DrawRectangleRec(enemyRect, RED);
}

void ballCollision(
    GameObject* go,
    GameContext* ctx
) {
    // # Collision
    auto ball = dynamic_pointer_cast<Ball>(go->rootNode);
    if (ball == nullptr) {
        return;
    }

    // # Collide and bounce
    for (auto j = 0; j < ctx->gos.size(); j++) {
        auto otherGo = ctx->gos[j];

        if (otherGo == go) {
            continue;
        }

        auto other = dynamic_pointer_cast<CharacterBody2D>(otherGo->rootNode);
        if (other == nullptr) {
            continue;
        }

        auto collision = CircleRectangleCollision(
            ball->position,
            ball->radius,
            other->position,
            other->size
        );

        if (collision.penetration > 0) {
            // # Resolve penetration
            ball->position = Vector2Add(
                ball->position,
                Vector2Scale(collision.normal, collision.penetration)
            );

            // # Resolve velocity
            // ## Calculate velocity separation
            float velocitySeparation = Vector2DotProduct(
                Vector2Subtract(ball->velocity, other->velocity),
                collision.normal
            );

            // ## Apply velocity separation
            ball->velocity = Vector2Add(
                ball->velocity,
                Vector2Scale(collision.normal, -2 * velocitySeparation)
            );
        }
    }
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

    auto playerRootNode = make_shared<Player>(
        (Vector2){ sixthScreen, screenHeight/2.0f },
        (Size){ 40.0f, 120.0f },
        (Vector2){ 0.0f, 0.0f },
        1.0f,
        10.0f
    );
    GameObject player {
        playerRootNode,
    };
    playerRootNode->AddNode(
        make_shared<RectangleView>(
            (Size){ 40.0f, 120.0f },
            BLUE
        )
    );
    playerRootNode->AddNode(
        make_shared<Collider>(
            ColliderType::Solid,
            Shape::Rectangle({ 40.0f, 120.0f }),
            (Vector2){ 0.0f, 0.0f }
        )
    );

    auto enemyRootNode = make_shared<Enemy>(
        (Vector2){ screenWidth - sixthScreen, screenHeight/2.0f },
        (Size){ 40.0f, 120.0f },
        (Vector2){ 0.0f, 0.0f },
        1.0f,
        10.0f
    );
    GameObject enemy {
        enemyRootNode,
    };
    enemyRootNode->AddNode(
        make_shared<RectangleView>(
            (Size){ 40.0f, 120.0f },
            RED
        )
    );
    enemyRootNode->AddNode(
        make_shared<Collider>(
            ColliderType::Solid,
            Shape::Rectangle({ 40.0f, 120.0f }),
            (Vector2){ 0.0f, 0.0f }
        )
    );

    float ballRadius = 15.0f;
    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * PI * 2;
    auto ballRootNode = make_shared<Ball>(
        ballRadius,
        (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
        (Size){ ballRadius*2, ballRadius*2 },
        (Vector2){ cos(randomAngle) * 5, sin(randomAngle) * 5 },
        5.0f
    );
    ballRootNode->AddNode(
        make_shared<CircleView>(
            ballRadius
        )
    );
    ballRootNode->AddNode(
        make_shared<Collider>(
            ColliderType::Solid,
            Shape::Circle(ballRadius),
            (Vector2){ 0.0f, 0.0f }
        )
    );

    GameObject ball {
        ballRootNode,
    };

    // # Field
    GameObject line {
        make_shared<LineView>(
            (Vector2){ screenWidth/2.0f, 80 },
            screenHeight - 160,
            WHITE,
            0.5f
        ),
    };

    GameObject circle {
        make_shared<CircleView>(
            80,
            (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
            WHITE,
            0.5f,
            false
        )
    };

    // # Game Context
    GameContext ctx = {
        { &ball, &player, &enemy, &line, &circle },
        screenWidth,
        screenHeight
    };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // # Initial
        for (auto i = 0; i < ctx.gos.size(); i++) {
            auto go = ctx.gos[i];
            traverseGameObjectUpdate(go, &ctx);
        }

        // # Calc collisions
        // for (auto i = 0; i < ctx.gos.size(); i++) {
        //     auto go = ctx.gos[i];
        //     ballCollision(go, &ctx);
        // }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (auto go: ctx.gos) {
                traverseGameObjectRender(go, &ctx);
            }
            debugger.Render(&ctx, nullptr);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}