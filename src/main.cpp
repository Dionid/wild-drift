#include <ctime>
#include "cengine/cengine.h"
#include "cengine/loader.h"
#include "utils.h"
#include "audio.h"
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

    cen::Debugger debugger;

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // # Audio
    InitAudioDevice();

    SpcAudio gameAudio = {
        Sound(
            LoadSound(cen::GetResourcePath("audio/start.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/hit.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/score.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/lost.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/win.wav").c_str())
        )
    };

    // # Scene
    cen::Scene scene;

    // # MatchEndMenu
    auto matchEndMenu = scene.node_storage->AddNode(std::make_unique<MatchEndMenu>());

    matchEndMenu->Deactivate();

    // # Match
    MatchManager* matchManager = scene.node_storage->AddNode(std::make_unique<MatchManager>(
        &gameAudio,
        [&]() {
            matchManager->Deactivate();
            matchEndMenu->playerWon = matchManager->playerScore > matchManager->enemyScore;
            matchEndMenu->Activate();
            EnableCursor();
        }
    ));

    matchManager->Deactivate();

    // # MainMenu

    MainMenu* mainMenu = scene.node_storage->AddNode(std::make_unique<MainMenu>(
        [&](cen::GameContext* ctx) {
            PlaySound(gameAudio.start);
            mainMenu->Deactivate();
            matchManager->Reset(ctx);
            matchManager->Activate();
            DisableCursor();
        }
    ));

    matchEndMenu->onRestart = [&](cen::GameContext* ctx) {
        PlaySound(gameAudio.start);
        matchEndMenu->Deactivate();
        matchManager->Reset(ctx);
        matchManager->Activate();
        DisableCursor();
    };

    // # Collision Engine
    cen::CollisionEngine collisionEngine;

    // # Game Context
    cen::GameContext ctx = {
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
            for (const auto& node: ctx.scene->node_storage->renderNodes) {
                if (node->AnyParentDeactivated()) {
                    continue;
                }
                node->TraverseRender(&ctx);
            }
            debugger.Render(&ctx);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseAudioDevice();

    // ## De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}