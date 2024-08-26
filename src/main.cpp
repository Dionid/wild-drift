#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include "engine.h"
#include "view.h"
#include "nodes.h"

using namespace std;

void CharacterApplyFriction(CharacterBody2D *c) {
    c->velocity.y *= .80f;
    c->velocity.x *= .80f;
}

void CharacterApplyWorldBoundaries(CharacterBody2D *c, float worldWidth, float worldHeight) {
    auto charNewPositionY = c->position.y + c->velocity.y;
    auto charNewPositionX = c->position.x + c->velocity.x;

    if (charNewPositionX - c->size.width/2 < 0) {
        charNewPositionX = c->size.width/2;
        c->velocity.x = 0;
    } else if (charNewPositionX + c->size.width/2 > worldWidth) {
        charNewPositionX = worldWidth - c->size.width/2;
        c->velocity.x = 0;
    }

    if (charNewPositionY - c->size.height/2 < 0) {
        charNewPositionY = c->size.height/2;
        c->velocity.y = 0;
    } else if (charNewPositionY + c->size.height/2 > worldHeight) {
        charNewPositionY = worldHeight - c->size.height/2;
        c->velocity.y = 0;
    }

    c->position.x = charNewPositionX;
    c->position.y = charNewPositionY;
}

void CharacterApplyVelocityToPosition(CharacterBody2D *c) {
    c->position.x += c->velocity.x;
    c->position.y += c->velocity.y;
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
        ) : CharacterBody2D(position, size, velocity)
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

    // # World Boundaries
    CharacterApplyWorldBoundaries(this, ctx->worldWidth, ctx->worldHeight);

    // # Field boundaries
    if (this->position.x + this->size.width/2 > ctx->worldWidth/2) {
        this->position.x = ctx->worldWidth/2 - this->size.width/2;
        this->velocity.x = 0;
    }

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(this);
}

void Player::Render(GameContext* ctx, GameObject* thisGO) {
    Rectangle playerRect = { this->position.x - this->size.width/2, this->position.y - this->size.height/2, this->size.width, this->size.height };
    DrawRectangleRec(playerRect, BLUE);
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

struct CollisionData {
    float penetration;
    Vector2 normal;
};

CollisionData CircleRectangleCollision(
    Vector2 circlePosition,
    float circleRadius,
    Vector2 rectPosition,
    Size rectSize
) {
    Vector2 closest = {
        fmaxf(rectPosition.x - rectSize.width/2, fminf(circlePosition.x, rectPosition.x + rectSize.width/2)),
        fmaxf(rectPosition.y - rectSize.height/2, fminf(circlePosition.y, rectPosition.y + rectSize.height/2))
    };

    Vector2 distance = Vector2Subtract(circlePosition, closest);
    float distanceLength = Vector2Length(distance);

    if (distanceLength < circleRadius) {
        return {
            circleRadius - distanceLength,
            Vector2Normalize(distance)
        };
    }

    return {
        0,
        Vector2Zero()
    };
}

void Ball::Update(GameContext* ctx, GameObject* thisGO) {
    auto worldWidth = ctx->worldWidth;
    auto worldHeight = ctx->worldHeight;
    auto gos = ctx->gos;

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(this);

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

    // # Collide and bounce
    for (auto go: gos) { 
        if (go == thisGO) {
            continue;
        }

        auto character = dynamic_pointer_cast<CharacterBody2D>(go->rootNode);
        if (character == nullptr) {
            continue;
        }

        auto collision = CircleRectangleCollision(
            this->position,
            this->radius,
            character->position,
            character->size
        );

        if (collision.penetration > 0) {
            // # Resolve penetration
            this->position = Vector2Add(
                this->position,
                Vector2Scale(collision.normal, collision.penetration)
            );

            // TODO: return this
            // # Add paddle velocity to ball
            // this->velocity = Vector2Add(
            //     this->velocity,
            //     Vector2Scale(character->velocity, 0.7f)
            // );

            // # Resolve velocity
            // ## Calculate velocity separation
            float velocitySeparation = Vector2DotProduct(
                Vector2Subtract(character->velocity, this->velocity),
                collision.normal
            );

            // ## Apply velocity separation
            this->velocity = Vector2Add(
                this->velocity,
                Vector2Scale(collision.normal, 2.0f * velocitySeparation)
            );
        }
    }

    // # Limit and normalize velocity
    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }
}

void Ball::Render(GameContext* ctx, GameObject* thisGO) {
    DrawCircleV(this->position, this->radius, WHITE);
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

        if (ball->position.x + ball->radius > this->position.x - this->size.width/2 + this->size.width/10) {
            directionX = 1;
        }
    }

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

    // # World Boundaries
    CharacterApplyWorldBoundaries(this, worldWidth, worldHeight);

    // # Field boundaries
    if (this->position.x - this->size.width/2 < worldWidth/2) {
        this->position.x = worldWidth/2 + this->size.width/2;
        this->velocity.x = 0;
    }

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(this);
}

void Enemy::Render(GameContext* ctx, GameObject* thisGO) {
    Rectangle enemyRect = { this->position.x - this->size.width/2, this->position.y - this->size.height/2, this->size.width, this->size.height };
    DrawRectangleRec(enemyRect, RED);
}

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    DisableCursor();
    SetTargetFPS(FPS);

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    const float sixthScreen = screenWidth/6.0f;

    // # Player

    GameObject player {
        make_shared<Player>(
            (Vector2){ sixthScreen, screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            .3f,
            10.0f
        )
    };

    GameObject enemy {
        make_shared<Enemy>(
            (Vector2){ screenWidth - sixthScreen, screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            .3f,
            10.0f
        )
    };

    float ballRadius = 15.0f;
    GameObject ball {
        make_shared<Ball>(
            ballRadius,
            (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
            (Size){ ballRadius*2, ballRadius*2 },
            (Vector2){ 3.0f, 0.0f },
            3.0f
        )
    };

    // ball.rootNode->nodes.push_back(
    //     make_shared<CircleView>(
    //         (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
    //         80,
    //         WHITE,
    //         0.5f,
    //         true
    //     )
    // );

    GameObject line {
        make_shared<LineView>(
            (Vector2){ screenWidth/2.0f, 80 },
            (Vector2){ screenWidth/2.0f, screenHeight - 80 },
            WHITE,
            0.5f
        ),
    };

    GameObject circle {
        make_shared<CircleView>(
            (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
            80,
            WHITE,
            0.5f,
            false
        )
    };

    // # Game Objects
    GameContext ctx = {
        { &player, &ball, &enemy, &line, &circle },
        screenWidth,
        screenHeight
    };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // # Initial
        for (auto go: ctx.gos) {
            GameObjectTraverseUpdate(go, &ctx);
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (auto go: ctx.gos) {
                GameObjectTraverseRender(go, &ctx);
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}