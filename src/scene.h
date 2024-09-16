#include "cengine/cengine.h"
#include "menus.h"
#include "match.h"
#include "step_lock_manager.h"
#include "events.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

class MainScene: public cen::Scene {
    public:
        SpcAudio* gameAudio;

        MainScene(
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine
        ): cen::Scene(
            screen,
            camera,
            renderingEngine
        ) {
            this->gameAudio = gameAudio;
        }

        void Init() override {
            // # Scene init
            // ## MatchEndMenu
            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>());

            matchEndMenu->Deactivate();

            // ## Match
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            matchManager->Deactivate();

            // ## MainMenu
            MainMenu* mainMenu = this->nodeStorage->AddNode(std::make_unique<MainMenu>());

            // # Events
            // ## StartEvent
            auto onStartEvent = [
                this,
                mainMenu,
                matchEndMenu,
                matchManager
            ](const cen::Event& event) {
                PlaySound(this->gameAudio->start);
                mainMenu->Deactivate();
                matchEndMenu->Deactivate();
                matchManager->Reset();
                matchManager->Activate();
                DisableCursor();
            };

            this->eventBus->on(
                StartEvent{},
                std::make_unique<cen::EventListener>(
                    onStartEvent
                )
            );

            this->eventBus->on(
                RestartEvent{},
                std::make_unique<cen::EventListener>(
                    onStartEvent
                )
            );

            // ## MatchEndEvent
            auto onMatchEndEvent = [
                this,
                mainMenu,
                matchEndMenu,
                matchManager
            ](const cen::Event& event) {
                matchManager->Deactivate();
                matchEndMenu->SetPlayerWon(matchManager->playerScore > matchManager->enemyScore);
                matchEndMenu->Activate();
                EnableCursor();
            };

            this->eventBus->on(
                OnMatchEndEvent{},
                std::make_unique<cen::EventListener>(
                    onMatchEndEvent
                )
            );

            // ## HostEvent
            auto onHostEvent = [
                this,
                mainMenu,
                matchEndMenu,
                matchManager
            ](const cen::Event& event) {
                // DisableCursor();
                // matchManager->InitMultiplayerMode(true);
                // PlaySound(this->gameAudio->start);
                // mainMenu->Deactivate();
                // matchEndMenu->Deactivate();
                // matchManager->Reset();
                // matchManager->Activate();
            };

            this->eventBus->on(
                HostEvent{},
                std::make_unique<cen::EventListener>(
                    onHostEvent
                )
            );
        }

        void RunSimulation() {
            // # Init scene
            this->Init();

            // ## Init Nodes
            for (const auto& node: this->nodeStorage->rootNodes) {
                node->TraverseInit();
            }

            // ## Node Storage
            this->nodeStorage->Init();

            // # Main Loop
            // ## Update
            const int targetFPS = 60;
            const std::chrono::milliseconds targetFrameTime(1000 / targetFPS);

            // ## Fixed update
            const int fixedUpdateRate = 40;
            const std::chrono::milliseconds targetFixedUpdateTime(1000 / fixedUpdateRate);
            int fixedUpdateCyclesLimit = 10;
            std::chrono::milliseconds accumulatedFixedTime(0);

            // # Init everything after sync
            auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();

            while (!WindowShouldClose())    // Detect window close button or ESC key
            {
                // # Frame Tick
                this->frameTick++;

                // # Input
                auto currentPlayerInput = cen::PlayerInput{
                    IsKeyDown(KEY_W),
                    IsKeyDown(KEY_S),
                    IsKeyDown(KEY_A),
                    IsKeyDown(KEY_D)
                };

                this->playerInputManager.currentPlayerInput = currentPlayerInput;

                // # Start
                auto frameStart = std::chrono::high_resolution_clock::now();

                // # Update
                this->nodeStorage->InitNewNodes();

                // # Fixed update
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
                lastFixedFrameTime = now;
                accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

                int fixedUpdateCycles = 0;
                while (accumulatedFixedTime >= targetFixedUpdateTime && fixedUpdateCycles < fixedUpdateCyclesLimit) {
                    this->simulationTick++;

                    // # Simulation current Tick
                    this->SimulationTick();

                    // ## Correct time and cycles
                    accumulatedFixedTime -= targetFixedUpdateTime;
                    fixedUpdateCycles++;
                }

                // # Initial
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseUpdate();
                }

                // # Flush events
                for (const auto& topic: this->topics) {
                    topic->flush();
                }

                this->eventBus->flush();

                // # Sync GameState and RendererState
                auto alpha = static_cast<double>(accumulatedFixedTime.count()) / targetFixedUpdateTime.count();

                this->renderingEngine->SyncRenderBuffer(
                    this->nodeStorage.get(),
                    alpha
                );

                // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
                // # End (busy wait)
                while (std::chrono::high_resolution_clock::now() - frameStart <= targetFrameTime) {}
            }
        }
};

#endif // MAIN_SCENE_H