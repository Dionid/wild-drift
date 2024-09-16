#include "cengine/cengine.h"
#include "game_tick.h"

class Server {
    public:
        cen::Scene* scene;
        cen::TickManager<GameStateTick>* tickManager;

        void SimulationTick() {
            this->scene->SimulationTick();
            tickManager->SaveGameTick(scene->playerInput);
        }

        void RunSimulation() {
            const int fixedUpdateRate = 40;
            const std::chrono::milliseconds targetFixedUpdateTime(1000 / fixedUpdateRate);
            auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();
            std::chrono::milliseconds accumulatedFixedTime(0);

            int fixedUpdateCyclesLimit = 10;

            while (true)    // Detect window close button or ESC key
            {
                // # Tick
                this->tickManager->currentTick++;

                // # Start
                auto frameStart = std::chrono::high_resolution_clock::now();

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
                    const auto& compareResult = this->tickManager->CompareArrivedAndPending();

                    if (
                        compareResult.invalidPendingGameStateTickId == -1
                    ) {
                        // ## Merge correct GameStateTick
                        this->tickManager->RemoveValidated(compareResult);
                    } else {
                        // ## Rollback and Apply
                        this->tickManager->Rollback(compareResult);

                        // ## Simulate new GameTicks using PlayerInputTicks
                        for (const auto& playerInputTick: this->tickManager->playerInputTicks) {
                            // TODO: think about playerInputTick update
                            // scene->playerInput = playerInputTick.input;
                            scene->SimulationTick();
                            this->tickManager->SaveGameTick(scene->playerInput);
                        }
                    }

                    // # Simulation current Tick
                    // scene->playerInput = currentPlayerInput;
                    scene->SimulationTick();
                    this->tickManager->SaveGameTick(scene->playerInput);

                    // ## Correct time and cycles
                    accumulatedFixedTime -= targetFixedUpdateTime;
                    fixedUpdateCycles++;
                }
                
                // QUESTION: Maybe remove this?
                // # Initial
                for (const auto& node: scene->nodeStorage->rootNodes) {
                    node->TraverseUpdate();
                }

                // # Flush events
                for (const auto& topic: scene->topics) {
                    topic->flush();
                }

                scene->eventBus->flush();

                // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
                // # End (busy wait)
                while (std::chrono::high_resolution_clock::now() - frameStart <= targetFixedUpdateTime) {}
            }
        }
};