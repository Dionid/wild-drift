#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
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

// # GO

class GameObject {
    public:
        virtual void FixedUpdate(float deltaTime, float worldWidth, float worldHeight) {};
        virtual void Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) {};
        virtual void Render(float deltaTime, float worldWidth, float worldHeight) {};
};

// # Character

struct Character {
    Vector2 position;
    Size size;
    Vector2 velocity;
    float speed;
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
class Player: public GameObject, public CharacterHolder {
    public:
        Player(Character c) {
            this->character = c;
        }

        void Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) override;
        void Render(float deltaTime, float worldWidth, float worldHeight) override;
};

// # Player Update function
void Player::Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) {
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

    // # Friction
    CharacterApplyFriction(&this->character);

    // # World Boundaries
    CharacterApplyWorldBoundaries(&this->character, worldWidth, worldHeight);

    // # Field boundaries
    // if (this->character.position.x + this->character.size.width/2 > worldWidth/2) {
    //     this->character.position.x = worldWidth/2 - this->character.size.width/2;
    //     this->character.velocity.x = 0;
    // }

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(&this->character);
}

void Player::Render(float deltaTime, float worldWidth, float worldHeight) {
    Rectangle playerRect = { this->character.position.x - this->character.size.width/2, this->character.position.y - this->character.size.height/2, this->character.size.width, this->character.size.height };
    DrawRectangleRec(playerRect, WHITE);
}

// # Ball
class Ball: public GameObject, public CharacterHolder {
    public:
        float radius;
        Ball(float radius, Character c) {
            this->character = c;
            this->radius = radius;
        }

        void Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) override;
        void Render(float deltaTime, float worldWidth, float worldHeight) override;
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

void Ball::Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) {
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
        if (go == this) {
            continue;
        }

        CharacterHolder *ch = dynamic_cast<CharacterHolder*>(go);
        if (ch == nullptr) {
            continue;
        }

        Character *c = &ch->character;

        auto collision = CircleRectangleCollision(
            this->character.position,
            this->radius,
            c->position,
            c->size
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
            //     Vector2Scale(c->velocity, 0.7f)
            // );

            // # Resolve velocity
            // ## Calculate velocity separation
            float velocitySeparation = Vector2DotProduct(
                Vector2Subtract(c->velocity, this->character.velocity),
                collision.normal
            );

            // ## Apply velocity separation
            this->character.velocity = Vector2Add(
                this->character.velocity,
                Vector2Scale(collision.normal, 2.0f * velocitySeparation)
            );
        }
    }

    // # Clamp velocity
    this->character.velocity = Vector2Clamp(
        this->character.velocity,
        (Vector2){-12.0f, -12.0f},
        (Vector2){12.0f, 12.0f}
    );
}

void Ball::Render(float deltaTime, float worldWidth, float worldHeight) {
    DrawCircleV(this->character.position, this->radius, WHITE);
}

// # Ball
class Enemy: public GameObject, public CharacterHolder {
    public:
        Enemy(Character c) {
            this->character = c;
        }

        void Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) override;
        void Render(float deltaTime, float worldWidth, float worldHeight) override;
};

void Enemy::Update(std::vector<GameObject*> gos, float deltaTime, float worldWidth, float worldHeight) {
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

void Enemy::Render(float deltaTime, float worldWidth, float worldHeight) {
    Rectangle enemyRect = { this->character.position.x - this->character.size.width/2, this->character.position.y - this->character.size.height/2, this->character.size.width, this->character.size.height };
    DrawRectangleRec(enemyRect, WHITE);
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
    Player player = {
        {
            (Vector2){ sixthScreen, screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f
        },
    };

    Enemy enemy = {
        {
            (Vector2){ screenWidth - sixthScreen, screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            3.0f
        }
    };

    float ballRadius = 15.0f;
    Ball ball = {
        ballRadius,
        {
            (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
            (Size){ ballRadius*2, ballRadius*2 },
            (Vector2){ 10.0f, 0.0f },
            3.0f
        }
    };

    // # Game Objects
    std::vector<GameObject*> gameObjects = { &player, &ball, &enemy };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // # Initial
        float deltaTime = DeltaTime();

        for (auto go: gameObjects) {
            go->Update(gameObjects, deltaTime, screenWidth, screenHeight);
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (auto go: gameObjects) {
                go->Render(deltaTime, screenWidth, screenHeight);
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