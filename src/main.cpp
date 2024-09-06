#include <ctime>
#include "cengine/cengine.h"
#include "utils.h"
#include "entity.h"
#include "match.h"
#include "menus.h"

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    EnableCursor();
    SetTargetFPS(FPS);

    Debugger debugger;

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // # Scene
    Scene scene;

    // # MatchEndMenu
    auto matchEndMenu = scene.node_storage->AddNode(std::make_unique<MatchEndMenu>());

    matchEndMenu->Deactivate();

    // # Match
    MatchManager* matchManager = scene.node_storage->AddNode(std::make_unique<MatchManager>(
        [&]() {
            matchManager->Deactivate();
            matchEndMenu->playerWon = matchManager->playerScore > matchManager->enemyScore;
            matchEndMenu->Activate();
            EnableCursor();
        },
        3
    ));

    matchManager->Deactivate();

    // # MainMenu

    MainMenu* mainMenu = scene.node_storage->AddNode(std::make_unique<MainMenu>(
        [&](GameContext* ctx) {
            mainMenu->Deactivate();
            matchManager->Reset(ctx);
            matchManager->Activate();
            DisableCursor();
        }
    ));

    matchEndMenu->onRestart = [&](GameContext* ctx) {
        matchEndMenu->Deactivate();
        matchManager->Reset(ctx);
        matchManager->Activate();
        DisableCursor();
    };

    // # Collision Engine
    CollisionEngine collisionEngine;

    // # Game Context
    GameContext ctx = {
        &scene,
        &collisionEngine,
        screenWidth,
        screenHeight
    };

    // # Init
    // # While nodes are initing more of them can be added
    for (const auto& node: ctx.scene->node_storage->nodes) {
        node->TraverseInit(&ctx);
    }

    // # Node Storage
    ctx.scene->node_storage->Init();

    // # Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // ## Update
        //----------------------------------------------------------------------------------
        scene.node_storage->InitNewNodes(&ctx);

        // ## Initial
        for (const auto& node: ctx.scene->node_storage->nodes) {
            node->TraverseUpdate(&ctx);
        }

        // ## Collision
        collisionEngine.NarrowCollisionCheckNaive(&ctx);

        //----------------------------------------------------------------------------------

        // ## Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (const auto& node: ctx.scene->node_storage->nodes) {
                node->TraverseRender(&ctx);
            }
            debugger.Render(&ctx);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // ## De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}