#include "cengine/cengine.h"
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

    // # Scene
    auto scene = std::make_unique<Scene>();

    // # Player
    const float sixthScreen = screenWidth/6.0f;

    auto player = scene.get()->node_storage->AddNode(
        Player::NewPlayer(
            (Vector2){ sixthScreen, screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f,
            10.0f
        )
    );

    float ballRadius = 15.0f;
    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * PI * 2;
    auto ball = scene.get()->node_storage->AddNode(
        Ball::NewBall(
            ballRadius,
            screenWidth,
            screenHeight,
            10.0f
        )
    );

    auto enemy = scene.get()->node_storage->AddNode(
        Enemy::NewEnemy(
            ball,
            (Vector2){ screenWidth - sixthScreen, screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f,
            10.0f
        )
    );

    // # Goals
    Size goalSize = { 15, screenHeight - 15 };

    auto leftGoal = Goal::NewGoal(
        true,
        (Vector2){ goalSize.width / 2 + 5, goalSize.height / 2 + 15 },
        goalSize
    );

    scene.get()->node_storage->AddNode(std::move(leftGoal));

    auto rightGoal = Goal::NewGoal(
        false,
        (Vector2){ screenWidth - goalSize.width / 2 - 5, goalSize.height / 2 + 15 },
        goalSize
    );

    scene.get()->node_storage->AddNode(std::move(rightGoal));

    // # Field
    scene.get()->node_storage->AddNode(
        std::make_unique<LineView>(
            (Vector2){ screenWidth/2.0f, 80 },
            screenHeight - 160,
            WHITE,
            0.5f
        )
    );

    scene.get()->node_storage->AddNode(
        std::make_unique<CircleView>(
            80,
            (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
            WHITE,
            0.5f,
            false
        )
    );

    // # Score Manager
    scene->node_storage->AddNode(
        std::make_unique<ScoreManager>(
            ball,
            player,
            enemy,
            0,
            0
        )
    );

    // # Collision Engine
    CollisionEngine collisionEngine;

    // # Game Context
    GameContext ctx = {
        scene.get(),
        &collisionEngine,
        screenWidth,
        screenHeight
    };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // # Initial
        for (const auto& node: ctx.scene->node_storage->nodes) {
            node->TraverseNodeUpdate(&ctx);
        }

        // # Collision
        collisionEngine.NarrowCollisionCheckNaive(&ctx);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (const auto& node: ctx.scene->node_storage->nodes) {
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