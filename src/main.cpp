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
#include "game_tick.h"

void simulationTick(
    cen::GameContext* ctx
) {
    // # Game Logic
    // ## Invalidate previous
    for (const auto& node: ctx->scene->nodeStorage->rootNodes) {
        node->TraverseInvalidatePrevious();
    }

    // ## Fixed Update
    for (const auto& node: ctx->scene->nodeStorage->rootNodes) {
        node->TraverseFixedUpdate(ctx);
    }

    // ## Collision
    ctx->scene->collisionEngine->NarrowCollisionCheckNaive(ctx->scene->nodeStorage.get());
}

void simulationPipeline(
    cen::GameContext* ctx,
    SpcAudio* gameAudio
) {
    // # Scene init

    // ## MatchEndMenu
    auto matchEndMenu = ctx->scene->nodeStorage->AddNode(std::make_unique<MatchEndMenu>(
        [&](cen::GameContext* ctx) {
            ctx->scene->eventBus.emit(RestartEvent());
        }
    ));

    matchEndMenu->Deactivate();

    // ## Match
    MatchManager* matchManager = ctx->scene->nodeStorage->AddNode(std::make_unique<MatchManager>(
        gameAudio,
        [&](cen::GameContext* ctx) {
            matchManager->Deactivate();
            matchEndMenu->SetPlayerWon(ctx, matchManager->playerScore > matchManager->enemyScore);
            matchEndMenu->Activate();
            EnableCursor();
        }
    ));

    matchManager->Deactivate();

    // ## MainMenu
    MainMenu* mainMenu = ctx->scene->nodeStorage->AddNode(std::make_unique<MainMenu>(
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

    // ## Init Nodes
    for (const auto& node: ctx->scene->nodeStorage->rootNodes) {
        node->TraverseInit(ctx);
    }

    // ## Node Storage
    ctx->scene->nodeStorage->Init();

    // ## Tick Manager
    SpcGameTickManager tickManager = SpcGameTickManager(ctx->scene->nodeStorage.get());

    // ## Main Loop
    const int targetFPS = 60;
    const int fixedUpdateRate = 40;

    const std::chrono::milliseconds targetFrameTime(1000 / targetFPS);
    const std::chrono::milliseconds targetFixedUpdateTime(1000 / fixedUpdateRate);

    auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds accumulatedFixedTime(0);

    int fixedUpdateCyclesLimit = 10;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        tickManager.currentTick++;

        // # Start
        auto frameStart = std::chrono::high_resolution_clock::now();

        // # Input
        auto currentPlayerInput = cen::PlayerInput{
            IsKeyDown(KEY_W),
            IsKeyDown(KEY_S),
            IsKeyDown(KEY_A),
            IsKeyDown(KEY_D)
        };

        // # Update
        ctx->scene->nodeStorage->InitNewNodes(ctx);

        // # Fixed update
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
        lastFixedFrameTime = now;
        accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

        int fixedUpdateCycles = 0;
        while (accumulatedFixedTime >= targetFixedUpdateTime && fixedUpdateCycles < fixedUpdateCyclesLimit) {
            // # Reconcile GameStateTick
            // ## Take arrived GameStateTick and check if they are correct
            const auto& compareResult = tickManager.CompareArrivedAndPending();

            if (
                compareResult.invalidPendingGameStateTickId == -1
            ) {
                // ## Merge correct GameStateTick
                tickManager.RemoveValidated(compareResult);
            } else {
                // ## Rollback and Apply
                tickManager.Rollback(compareResult);

                // ## Simulate new GameTicks using PlayerInputTicks
                for (const auto& playerInputTick: tickManager.playerInputTicks) {
                    ctx->playerInput = playerInputTick.input;
                    simulationTick(ctx);
                    tickManager.SaveGameTick(ctx->playerInput);
                }
            }

            // # Simulation current Tick
            ctx->playerInput = currentPlayerInput;
            simulationTick(ctx);
            tickManager.SaveGameTick(ctx->playerInput);

            // ## Correct time and cycles
            accumulatedFixedTime -= targetFixedUpdateTime;
            fixedUpdateCycles++;
        }

        // # Initial
        for (const auto& node: ctx->scene->nodeStorage->rootNodes) {
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
            ctx->scene->nodeStorage.get(),
            alpha
        );

        // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
        // # End (busy wait)
        while (std::chrono::high_resolution_clock::now() - frameStart <= targetFrameTime) {}
    }
};

void renderingPipeline(cen::RenderingEngine2D* renderingEngine, cen::Debugger* debugger) {
    renderingEngine->runPipeline(debugger);
}

int multiplayerServerPipeline(cen::MultiplayerManager* multiplayerManager) {
    return multiplayerManager->runServerPipeline();
}

int multiplayerClientPipeline(cen::MultiplayerManager* multiplayerManager) {
    return multiplayerManager->runClientPipeline();
}

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
    cen::CollisionEngine collisionEngine;

    cen::Scene scene = cen::Scene(
        &camera,
        &collisionEngine
    );

    // # Game Context
    auto input = cen::PlayerInput{};

    cen::GameContext ctx = {
        &scene,
        input,
        screenWidth,
        screenHeight
    };

    cen::MultiplayerManager multiplayerManager;

    // # Game Loop Thread
    std::vector<std::thread> threads;

    threads.push_back(std::thread(simulationPipeline, &ctx, &gameAudio));

    // # Server Thread
    // bool isServer;
    // std::cout << "Are you the server? (1/0): ";
    // std::cin >> isServer;
    // if (isServer) {
    //     threads.push_back(std::thread(multiplayerServerPipeline, &multiplayerManager));
    // } else {
    //     threads.push_back(std::thread(multiplayerClientPipeline, &multiplayerManager));
    // }

    // # Render Loop Thread
    renderingPipeline(scene.renderingEngine.get(), &debugger);

    // # Exit
    // ## Join threads after stop signal
    for (auto& thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // ## Audio
    CloseAudioDevice();

    // ## Close window
    CloseWindow();

    return 0;
}