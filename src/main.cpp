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
        virtual void Update(float deltaTime, float worldWidth, float worldHeight) {};
        virtual void Render(float deltaTime, float worldWidth, float worldHeight) {};
};

// # Character

struct Character {
    Vector2 position;
    Size size;
    Vector2 velocity;
    float speed;
};

void CharacterApplyFriction(Character *c) {
    c->velocity.y *= .80f;
    c->velocity.x *= .80f;
}

void CharacterApplyWorldBoundaries(Character *c, float worldWidth, float worldHeight) {
    auto playerNewPositionY = c->position.y + c->velocity.y;
    auto playerNewPositionX = c->position.x + c->velocity.x;

    if (playerNewPositionX - c->size.width/2 < 0) {
        playerNewPositionX = c->size.width/2;
        c->velocity.x = 0;
    } else if (playerNewPositionX + c->size.width/2 > worldWidth) {
        playerNewPositionX = worldWidth - c->size.width/2;
        c->velocity.x = 0;
    }

    if (playerNewPositionY - c->size.height/2 < 0) {
        playerNewPositionY = c->size.height/2;
        c->velocity.y = 0;
    } else if (playerNewPositionY + c->size.height/2 > worldHeight) {
        playerNewPositionY = worldHeight - c->size.height/2;
        c->velocity.y = 0;
    }

    c->position.x = playerNewPositionX;
    c->position.y = playerNewPositionY;
}

void CharacterApplyVelocityToPosition(Character *c) {
    c->position.x += c->velocity.x;
    c->position.y += c->velocity.y;
}

// # Player
class Player: public GameObject {
    public:
        Character character;
        Player(Character c) {
            this->character = c;
        }

        void Update(float deltaTime, float worldWidth, float worldHeight) override;
        void Render(float deltaTime, float worldWidth, float worldHeight) override;
};

// # Player Update function
void Player::Update(float deltaTime, float worldWidth, float worldHeight) {
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
    if (this->character.position.x + this->character.size.width/2 > worldWidth/2) {
        this->character.position.x = worldWidth/2 - this->character.size.width/2;
        this->character.velocity.x = 0;
    }

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(&this->character);
}

void Player::Render(float deltaTime, float worldWidth, float worldHeight) {
    Rectangle playerRect = { this->character.position.x - this->character.size.width/2, this->character.position.y - this->character.size.height/2, this->character.size.width, this->character.size.height };
    DrawRectangleRec(playerRect, BLUE);
}

// # Ball
class Ball: public GameObject {
    public:
        float radius;
        Character character;
        Ball(float radius, Character c) {
            this->character = c;
            this->radius = radius;
        }

        void Update(float deltaTime, float worldWidth, float worldHeight) override;
        void Render(float deltaTime, float worldWidth, float worldHeight) override;
};

void Ball::Update(float deltaTime, float worldWidth, float worldHeight) {
    // # World Boundaries
    CharacterApplyWorldBoundaries(&this->character, worldWidth, worldHeight);

    // # Velocity -> Position
    CharacterApplyVelocityToPosition(&this->character);
}

void Ball::Render(float deltaTime, float worldWidth, float worldHeight) {
    DrawCircleV(this->character.position, this->radius, WHITE);
}

// # Ball
class Enemy: public GameObject {
    public:
        Character character;
        Enemy(Character c) {
            this->character = c;
        }

        void Update(float deltaTime, float worldWidth, float worldHeight) override;
        void Render(float deltaTime, float worldWidth, float worldHeight) override;
};

void Enemy::Update(float deltaTime, float worldWidth, float worldHeight) {
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
            (Vector2){ 1.0f, 0.0f },
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
            go->Update(deltaTime, screenWidth, screenHeight);
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(LIGHTGRAY);
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