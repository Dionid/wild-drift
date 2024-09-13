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
    cen::Scene* scene
) {
    // # Game Logic
    // ## Invalidate previous
    for (const auto& node: scene->nodeStorage->rootNodes) {
        node->TraverseInvalidatePrevious();
    }

    // ## Fixed Update
    for (const auto& node: scene->nodeStorage->rootNodes) {
        node->TraverseFixedUpdate();
    }

    // ## Collision
    scene->collisionEngine->NarrowCollisionCheckNaive(scene->nodeStorage.get());
}

void simulationPipeline(
    cen::Scene* scene,
    SpcAudio* gameAudio
) {
    // # Scene init
    // ## MatchEndMenu
    auto matchEndMenu = scene->nodeStorage->AddNode(std::make_unique<MatchEndMenu>(
        [&]() {
            scene->eventBus->emit(RestartEvent());
        }
    ));

    matchEndMenu->Deactivate();

    // ## Match
    MatchManager* matchManager = scene->nodeStorage->AddNode(std::make_unique<MatchManager>(
        gameAudio,
        [&]() {
            matchManager->Deactivate();
            matchEndMenu->SetPlayerWon(matchManager->playerScore > matchManager->enemyScore);
            matchEndMenu->Activate();
            EnableCursor();
        }
    ));

    matchManager->Deactivate();

    // ## MainMenu
    MainMenu* mainMenu = scene->nodeStorage->AddNode(std::make_unique<MainMenu>(
        [&]() {
            scene->eventBus->emit(StartEvent());
        }
    ));

    auto ose = cen::EventListener(
        [&](const cen::Event& event) {
            PlaySound(gameAudio->start);
            mainMenu->Deactivate();
            matchEndMenu->Deactivate();
            matchManager->Reset();
            matchManager->Activate();
            DisableCursor();
        }
    );

    scene->eventBus->on(
        StartEvent{},
        &ose
    );

    scene->eventBus->on(
        RestartEvent{},
        &ose
    );

    // ## Init Nodes
    for (const auto& node: scene->nodeStorage->rootNodes) {
        node->TraverseInit();
    }

    // ## Node Storage
    scene->nodeStorage->Init();

    // ## Tick Manager
    SpcGameTickManager tickManager = SpcGameTickManager(scene->nodeStorage.get());

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
        scene->nodeStorage->InitNewNodes();

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
                    // ctx->playerInput = playerInputTick.input;
                    scene->playerInput = playerInputTick.input;
                    simulationTick(scene);
                    // tickManager.SaveGameTick(ctx->playerInput);
                }
            }

            // # Simulation current Tick
            scene->playerInput = currentPlayerInput;
            simulationTick(scene);

            // ## Correct time and cycles
            accumulatedFixedTime -= targetFixedUpdateTime;
            fixedUpdateCycles++;
        }

        // # Initial
        for (const auto& node: scene->nodeStorage->rootNodes) {
            node->TraverseUpdate();
        }

        // # Flush events
        for (const auto& topic: scene->topics) {
            topic->flush();
        }

        scene->eventBus->flush();

        // # Map GameState to RendererState
        auto alpha = static_cast<double>(accumulatedFixedTime.count()) / targetFixedUpdateTime.count();

        scene->renderingEngine->SyncRenderBuffer(
            scene->nodeStorage.get(),
            alpha
        );

        // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
        // # End (busy wait)
        while (std::chrono::high_resolution_clock::now() - frameStart <= targetFrameTime) {}
    }
};

void renderingPipeline(cen::Scene* scene) {
    scene->renderingEngine->runPipeline(&scene->debugger);
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
        cen::ScreenResolution{screenWidth, screenHeight},
        &camera,
        cen::Debugger{}
    );

    cen::MultiplayerManager multiplayerManager;

    // # Game Loop Thread
    std::vector<std::thread> threads;

    threads.push_back(std::thread(simulationPipeline, &scene, &gameAudio));

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
    renderingPipeline(&scene);

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