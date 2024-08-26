#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include "engine.h"
using namespace std;

// # Types

struct Size {
    float width;
    float height;
};

// # Delta
const float FPS = 60.0f;
const float secondsPerFrame = 1.0f / FPS;

float DeltaTime() {
    return GetFrameTime() / secondsPerFrame;
}

// # Character

struct Character {
    Vector2 position;
    Size size;
    Vector2 velocity;
    float speed;
    float maxVelocity;
};

class CharacterHolder {
    public:
        Character character;
};

void CharacterApplyFriction(Character *c) {
    c->velocity.y *= .80f;
    c->velocity.x *= .80f;
}

void CharacterApplyWorldBoundaries(Character *c, float worldWidth, float worldHeight) {
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

void CharacterApplyVelocityToPosition(Character *c) {
    c->position.x += c->velocity.x;
    c->position.y += c->velocity.y;
}

// # Player
class Player: public Component, public CharacterHolder {
    public:
        Player(Character c) {
            this->character = c;
        }

        void Update(GameContext ctx, GameObject* thisGO) override;
        void Render(GameContext ctx, GameObject* thisGO) override;
};

// # Player Update function
void Player::Update(GameContext ctx, GameObject* thisGO) {
    float deltaTime = DeltaTime();

    // # Calc velocity
    auto directionY = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    auto directionX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

    Vector2 newSpeed = Vector2Scale(
        Vector2Normalize({
            this->character.speed * directionX,
            this->character.speed * directionY,
        }),
        this->character.speed
    );

    this->character.velocity.y += newSpeed.y * deltaTime;
    this->character.velocity.x += newSpeed.x * deltaTime;

    if (Vector2Length(this->character.velocity) > this->character.maxVelocity) {
        this->character.velocity = Vector2Scale(Vector2Normalize(this->character.velocity), this->character.maxVelocity);
    }

    // # Friction
    CharacterApplyFriction(&this->character);

    // # World Boundaries
    CharacterApplyWorldBoundaries(&this->character, ctx.worldWidth, ctx.worldHeight);

    // # Field boundaries
    if (this->character.position.x + this->character.size.width/2 > ctx.worldWidth/2) {
        this->character.position.x = ctx.worldWidth/2 - this->character.size.width/2;
        this->character.velocity.x = 0;
    }

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(&this->character);
}

void Player::Render(GameContext ctx, GameObject* thisGO) {
    Rectangle playerRect = { this->character.position.x - this->character.size.width/2, this->character.position.y - this->character.size.height/2, this->character.size.width, this->character.size.height };
    DrawRectangleRec(playerRect, BLUE);
}

// # Ball
class Ball: public Component, public CharacterHolder {
    public:
        float radius;
        Ball(float radius, Character c) {
            this->character = c;
            this->radius = radius;
        }

        void Update(GameContext ctx, GameObject* thisGO) override;
        void Render(GameContext ctx, GameObject* thisGO) override;
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

void Ball::Update(GameContext ctx, GameObject* thisGO) {
    auto worldWidth = ctx.worldWidth;
    auto worldHeight = ctx.worldHeight;
    auto gos = ctx.gos;

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(&this->character);

    // # World Boundaries
    if (this->character.position.x - this->radius < 0) {
        this->character.position.x = this->radius;
        this->character.velocity = Vector2Reflect(this->character.velocity, (Vector2){1, 0});
    } else if (this->character.position.x + this->radius > worldWidth) {
        this->character.position.x = worldWidth - this->radius;
        this->character.velocity = Vector2Reflect(this->character.velocity, (Vector2){-1, 0});
    }

    if (this->character.position.y - this->radius < 0) {
        this->character.position.y = this->radius;
        this->character.velocity = Vector2Reflect(this->character.velocity, (Vector2){0, -1});
    } else if (this->character.position.y + this->radius > worldHeight) {
        this->character.position.y = worldHeight - this->radius;
        this->character.velocity = Vector2Reflect(this->character.velocity, (Vector2){0, 1});
    }

    // # Collide and bounce
    for (auto go: gos) { 
        if (go == thisGO) {
            continue;
        }

        for (auto c: go->components) {
            std::shared_ptr<CharacterHolder> ch = dynamic_pointer_cast<CharacterHolder>(c);
            if (ch == nullptr) {
                continue;
            }

            Character *character = &ch->character;

            auto collision = CircleRectangleCollision(
                this->character.position,
                this->radius,
                character->position,
                character->size
            );

            if (collision.penetration > 0) {
                // # Resolve penetration
                this->character.position = Vector2Add(
                    this->character.position,
                    Vector2Scale(collision.normal, collision.penetration)
                );

                // TODO: return this
                // # Add paddle velocity to ball
                // this->character.velocity = Vector2Add(
                //     this->character.velocity,
                //     Vector2Scale(character->velocity, 0.7f)
                // );

                // # Resolve velocity
                // ## Calculate velocity separation
                float velocitySeparation = Vector2DotProduct(
                    Vector2Subtract(character->velocity, this->character.velocity),
                    collision.normal
                );

                // ## Apply velocity separation
                this->character.velocity = Vector2Add(
                    this->character.velocity,
                    Vector2Scale(collision.normal, 2.0f * velocitySeparation)
                );
            }
        }
    }

    // # Limit and normalize velocity
    if (Vector2Length(this->character.velocity) > this->character.maxVelocity) {
        this->character.velocity = Vector2Scale(Vector2Normalize(this->character.velocity), this->character.maxVelocity);
    }
}

void Ball::Render(GameContext ctx, GameObject* thisGO) {
    DrawCircleV(this->character.position, this->radius, WHITE);
}

// # Ball
class Enemy: public Component, public CharacterHolder {
    public:
        Enemy(Character c) {
            this->character = c;
        }

        void Update(GameContext ctx, GameObject* thisGO) override;
        void Render(GameContext ctx, GameObject* thisGO) override;
};

void Enemy::Update(GameContext ctx, GameObject* thisGO) {
    auto worldWidth = ctx.worldWidth;
    auto worldHeight = ctx.worldHeight;
    auto gos = ctx.gos;
    float deltaTime = DeltaTime();

    float directionX = 0;
    float directionY = 0;

    for (auto go: gos) {
        if (go == thisGO) {
            continue;
        }

        for (auto c: go->components) {
            std::shared_ptr<Ball> ball = dynamic_pointer_cast<Ball>(c);
            if (ball == nullptr) {
                continue;
            }

            if (this->character.position.y > ball->character.position.y + ball->radius + 50) {
                directionY = -1;
            } else if (this->character.position.y < ball->character.position.y - ball->radius - 50) {
                directionY = 1;
            }

            if (ball->character.position.x < worldWidth/2) {
                directionX = 1;
            } else {
                directionX = -1;
            }

            if (ball->character.position.x + ball->radius > this->character.position.x - this->character.size.width/2 + this->character.size.width/10) {
                directionX = 1;
            }
        }
    }

    Vector2 newSpeed = Vector2Scale(
        Vector2Normalize({
            this->character.speed * directionX,
            this->character.speed * directionY,
        }),
        this->character.speed
    );

    this->character.velocity.y += newSpeed.y * deltaTime;
    this->character.velocity.x += newSpeed.x * deltaTime;

    if (Vector2Length(this->character.velocity) > this->character.maxVelocity) {
        this->character.velocity = Vector2Scale(Vector2Normalize(this->character.velocity), this->character.maxVelocity);
    }

    // # Friction
    CharacterApplyFriction(&this->character);

    // # World Boundaries
    CharacterApplyWorldBoundaries(&this->character, worldWidth, worldHeight);

    // # Field boundaries
    if (this->character.position.x - this->character.size.width/2 < worldWidth/2) {
        this->character.position.x = worldWidth/2 + this->character.size.width/2;
        this->character.velocity.x = 0;
    }

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(&this->character);
}

void Enemy::Render(GameContext ctx, GameObject* thisGO) {
    Rectangle enemyRect = { this->character.position.x - this->character.size.width/2, this->character.position.y - this->character.size.height/2, this->character.size.width, this->character.size.height };
    DrawRectangleRec(enemyRect, RED);
}

class Line: public Component {
    public:
        Vector2 start;
        Vector2 end;
        float alpha;
        Color color;

        Line(Vector2 start, Vector2 end, Color color = WHITE, float alpha = 1.0f) {
            this->start = start;
            this->end = end;
            this->alpha = alpha;
            this->color = color;
        }

        void Render(GameContext ctx, GameObject* thisGO) override {
            DrawLineV(this->start, this->end, ColorAlpha(this->color, this->alpha));
        }
};

class Circle: public Component {
    public:
        Vector2 center;
        float radius;
        float alpha;
        Color color;


        Circle(Vector2 center, float radius, Color color = WHITE, float alpha = 1.0f) {
            this->center = center;
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
        }

        void Render(GameContext ctx, GameObject* thisGO) override {
            DrawCircleLinesV(this->center, this->radius, ColorAlpha(this->color, this->alpha));
        }
};

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
    GameObject player;

    player.components.push_back(
        make_shared<Player>(
            (Character){
                (Vector2){ sixthScreen, screenHeight/2.0f },
                (Size){ 40.0f, 120.0f },
                (Vector2){ 0.0f, 0.0f },
                .3f,
                10.0f
            }
        )
    );

    GameObject enemy;

    enemy.components.push_back(
        make_shared<Enemy>(
            (Character){
                (Vector2){ screenWidth - sixthScreen, screenHeight/2.0f },
                (Size){ 40.0f, 120.0f },
                (Vector2){ 0.0f, 0.0f },
                .3f,
                10.0f
            }
        )
    );

    float ballRadius = 15.0f;
    GameObject ball;

    ball.components.push_back(
        make_shared<Ball>(
            ballRadius,
            (Character){
                (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
                (Size){ ballRadius*2, ballRadius*2 },
                (Vector2){ 3.0f, 0.0f },
                3.0f,
                3.0f
            }
        )
    );

    GameObject line;

    line.components.push_back(
        make_shared<Line>(
            (Vector2){ screenWidth/2.0f, 80 },
            (Vector2){ screenWidth/2.0f, screenHeight - 80 },
            WHITE,
            0.5f
        )
    );

    GameObject circle;

    circle.components.push_back(
        make_shared<Circle>(
            (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
            80,
            WHITE,
            0.5f
        )
    );

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
            for (auto component: go->components) {
                component->Update(ctx, go);
            }
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (auto go: ctx.gos) {
                for (auto component: go->components) {
                    component->Render(ctx, go);
                }
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