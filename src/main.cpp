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
    cen::Scene scene = cen::Scene();

    // # MatchEndMenu
    auto matchEndMenu = scene.node_storage->AddNode(std::make_unique<MatchEndMenu>(
        [&](cen::GameContext* ctx) {
            scene.eventBus.emit(RestartEvent());
        }
    ));

    matchEndMenu->Deactivate();

    // # Match
    MatchManager* matchManager = scene.node_storage->AddNode(std::make_unique<MatchManager>(
        &gameAudio,
        [&](cen::GameContext* ctx) {
            matchManager->Deactivate();
            matchEndMenu->SetPlayerWon(ctx, matchManager->playerScore > matchManager->enemyScore);
            matchEndMenu->Activate();
            EnableCursor();
        }
    ));

    matchManager->Deactivate();

    // # MainMenu
    MainMenu* mainMenu = scene.node_storage->AddNode(std::make_unique<MainMenu>(
        [&](cen::GameContext* ctx) {
            scene.eventBus.emit(StartEvent());
        }
    ));

    auto ose = cen::EventListener(
        [&](cen::GameContext* ctx, const cen::Event& event) {
            PlaySound(gameAudio.start);
            mainMenu->Deactivate();
            matchEndMenu->Deactivate();
            matchManager->Reset(ctx);
            matchManager->Activate();
            DisableCursor();
        }
    );

    scene.eventBus.on(
        StartEvent{},
        &ose
    );

    scene.eventBus.on(
        RestartEvent{},
        &ose
    );

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

        // ## Map Game state to Renderer
        scene.renderingEngine->MapNodesToCanvasItems();

        //----------------------------------------------------------------------------------

        // ## Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            scene.renderingEngine->Render(&ctx);
            debugger.Render(&ctx);
        EndDrawing();
        //----------------------------------------------------------------------------------

        // for (const auto& topic: ctx.scene->topics) {
        //     topic->flush();
        // }

        scene.eventBus.flush(&ctx);
    }

    CloseAudioDevice();

    // ## De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}