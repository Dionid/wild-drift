#include <raylib.h>
#include "view.h"
#include "debug.h"
#include "utils.h"
#include "entity.h"

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
    auto line = std::make_shared<LineView>(
        (Vector2){ screenWidth/2.0f, 80 },
        screenHeight - 160,
        WHITE,
        0.5f
    );

    auto circle = std::make_shared<CircleView>(
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