#include <ctime>
#include <thread>
#include <chrono>
#include "cengine/cengine.h"
#include "cengine/loader.h"
#include "utils.h"
#include "audio.h"
#include "entity.h"
#include "match.h"
#include "menus.h"

void gameLoopPipeline(
    cen::GameContext* ctx,
    SpcAudio* gameAudio
) {
    // # MatchEndMenu
    auto matchEndMenu = ctx->scene->node_storage->AddNode(std::make_unique<MatchEndMenu>(
        [&](cen::GameContext* ctx) {
            ctx->scene->eventBus.emit(RestartEvent());
        }
    ));

    matchEndMenu->Deactivate();

    // # Match
    MatchManager* matchManager = ctx->scene->node_storage->AddNode(std::make_unique<MatchManager>(
        gameAudio,
        [&](cen::GameContext* ctx) {
            matchManager->Deactivate();
            matchEndMenu->SetPlayerWon(ctx, matchManager->playerScore > matchManager->enemyScore);
            matchEndMenu->Activate();
            EnableCursor();
        }
    ));

    matchManager->Deactivate();

    // # MainMenu
    MainMenu* mainMenu = ctx->scene->node_storage->AddNode(std::make_unique<MainMenu>(
        [&](cen::GameContext* ctx) {
            ctx->scene->eventBus.emit(StartEvent());
        }
    ));

    auto ose = cen::EventListener(
        [&](cen::GameContext* ctx, const cen::Event& event) {
            PlaySound(gameAudio->start);
            mainMenu->Deactivate();
            matchEndMenu->Deactivate();
            matchManager->Reset(ctx);
            matchManager->Activate();
            DisableCursor();
        }
    );

    ctx->scene->eventBus.on(
        StartEvent{},
        &ose
    );

    ctx->scene->eventBus.on(
        RestartEvent{},
        &ose
    );

    // # Init
    // # While nodes are initing more of them can be added
    for (const auto& node: ctx->scene->node_storage->nodes) {
        node->TraverseInit(ctx);
    }

    // # Node Storage
    ctx->scene->node_storage->Init();

    // # Main Loop
    const int targetFPS = 60;
    const int fixedUpdateRate = 40;

    const std::chrono::milliseconds targetFrameTime(1000 / targetFPS);
    const std::chrono::milliseconds targetFixedUpdateTime(1000 / fixedUpdateRate);

    auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds accumulatedFixedTime(0);

    int fixedUpdateCyclesLimit = 10;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // # Start
        auto frameStart = std::chrono::high_resolution_clock::now();

        // # Update
        ctx->scene->node_storage->InitNewNodes(ctx);

        // # Fixed update
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
        lastFixedFrameTime = now;
        accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

        int fixedUpdateCycles = 0;
        while (accumulatedFixedTime >= targetFixedUpdateTime && fixedUpdateCycles < fixedUpdateCyclesLimit) {
            // # Invalidate previous
            for (const auto& node: ctx->scene->node_storage->nodes) {
                node->TraverseInvalidatePrevious();
            }

            for (const auto& node: ctx->scene->node_storage->nodes) {
                node->TraverseFixedUpdate(ctx);
            }

            // ## Collision
            ctx->collisionEngine->NarrowCollisionCheckNaive(ctx);

            accumulatedFixedTime -= targetFixedUpdateTime;
            fixedUpdateCycles++;
        }

        // # Initial
        for (const auto& node: ctx->scene->node_storage->nodes) {
            node->TraverseUpdate(ctx);
        }

        // # Flush events
        for (const auto& topic: ctx->scene->topics) {
            topic->flush();
        }

        ctx->scene->eventBus.flush(ctx);

        // # Map GameState to RendererState
        auto alpha = static_cast<double>(accumulatedFixedTime.count()) / targetFixedUpdateTime.count();

        ctx->scene->renderingEngine->SyncRenderBuffer(
            ctx->scene->node_storage.get(),
            alpha
        );

        // # End
        auto frameEnd = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> frameDuration = frameEnd - frameStart;

        // QUESTION: maybe sleep better? But it overshoot for 3ms
        // Busy wait
        while (std::chrono::high_resolution_clock::now() - frameStart <= targetFrameTime) {}
    }
};

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

    cen::CollisionEngine collisionEngine;

    // # Game Context
    cen::GameContext ctx = {
        &scene,
        &collisionEngine,
        screenWidth,
        screenHeight
    };

    // # Game Loop Thread
    std::thread gameLoopThread(gameLoopPipeline, &ctx, &gameAudio);

    // # Render Loop Thread
    int counter = 0;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // std::cout << GetFrameTime() << std::endl;
        BeginDrawing();
            ClearBackground(BLACK);
            scene.renderingEngine->Render();
            debugger.Render(&ctx);
        EndDrawing();
    }

    gameLoopThread.join();

    CloseAudioDevice();

    // ## De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}