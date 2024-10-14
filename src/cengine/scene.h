#ifndef CENGINE_SCENE_H
#define CENGINE_SCENE_H

#include <vector>
#include <map>
#include <atomic>
#include <functional>
#include "rendering.h"
#include "node_storage.h"
#include "collision.h"
#include "event.h"

namespace cen {
    typedef std::string scene_name;

    class Scene {
        public:
            bool isInitialized = false;
            std::atomic<bool> isAlive = true;
            scene_name name;

            u_int64_t frameTick;
            u_int64_t fixedFrameTick;

            int simulationFrameRate;
            std::chrono::milliseconds simulationFrameRateInMs;
            int fixedSimulationFrameRate;
            std::chrono::milliseconds fixedSimulationFrameRateInMs;
            int fixedSimulationFrameCyclesLimit;

            Camera2D* camera;
            cen::ScreenResolution screen;
            // TODO: change this to pointer
            cen::PlayerInputManager playerInputManager;
            cen::RenderingEngine2D* renderingEngine;
            cen::EventBus eventBus;

            std::unique_ptr<cen::CollisionEngine> collisionEngine;
            std::unique_ptr<cen::NodeStorage> nodeStorage;

            Scene(
                scene_name name,
                cen::ScreenResolution screen,
                Camera2D* camera,
                RenderingEngine2D* renderingEngine,
                cen::EventBus* eventBus,
                int simulationFrameRate = 60,
                int simulationFixedFrameRate = 40,
                int fixedSimulationFrameCyclesLimit = 10,
                cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{},
                std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
                std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>(),
                uint64_t frameTick = 0,
                uint64_t simulationTick = 0
            ): eventBus(eventBus) {
                this->name = name;
                this->screen = screen;
                this->camera = camera;
                this->renderingEngine = renderingEngine;
                this->playerInputManager = playerInputManager;
                this->collisionEngine = std::move(collisionEngine);
                this->nodeStorage = std::move(nodeStorage);
                this->nodeStorage->scene = this;

                this->simulationFrameRate = simulationFrameRate;
                this->fixedSimulationFrameRate = simulationFixedFrameRate;

                this->simulationFrameRateInMs = std::chrono::milliseconds(1000 / simulationFrameRate);
                this->fixedSimulationFrameRateInMs = std::chrono::milliseconds(1000 / simulationFixedFrameRate);
                this->fixedSimulationFrameCyclesLimit = fixedSimulationFrameCyclesLimit;
            }

            virtual ~Scene() {};

            virtual void Init() {};
            virtual void Run() {};
            virtual void BeforeStop() {};
            virtual void AfterStop() {};
            
            virtual void Stop() {
                this->BeforeStop();
                this->isAlive.store(false, std::memory_order_release);
            };

            void FullInit() {
                // # Init scene
                this->Init();

                // ## Init Nodes
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseInit();
                }

                // ## Node Storage
                this->nodeStorage->Init();

                this->isInitialized = true;
            }

            void FixedSimulationTick() {
                // # Invalidate previous
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseInvalidatePrevious();
                }

                // # Fixed Update
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseFixedUpdate();
                }

                // # Collision
                this->collisionEngine->NarrowCollisionCheckNaive(this->nodeStorage.get());
            }

            template <typename T>
            T* AddNode(std::unique_ptr<T> newNode) {
                return this->nodeStorage->AddNode(std::move(newNode));
            }
    };

    class LocalScene: public Scene {
        public:

            LocalScene(
                scene_name name,
                cen::ScreenResolution screen,
                Camera2D* camera,
                RenderingEngine2D* renderingEngine,
                cen::EventBus* eventBus,
                int simulationFrameRate = 60,
                int simulationFixedFrameRate = 40,
                int fixedSimulationFrameCyclesLimit = 10,
                cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{},
                std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
                std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>(),
                uint64_t frameTick = 0,
                uint64_t simulationTick = 0
            ): Scene(
                name,
                screen,
                camera,
                renderingEngine,
                eventBus,
                simulationFrameRate,
                simulationFixedFrameRate,
                fixedSimulationFrameCyclesLimit,
                playerInputManager,
                std::move(collisionEngine),
                std::move(nodeStorage),
                frameTick,
                simulationTick
            ) {}

            virtual ~LocalScene() {}

            void Run() override {
                if (!this->isInitialized) {
                    // # Init scene
                    this->FullInit();
                }

                std::chrono::milliseconds accumulatedFixedTime(0);
                auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();

                while (this->isAlive.load(std::memory_order_acquire))    // Detect window close button or ESC key
                {
                    // # Start
                    auto frameStart = std::chrono::high_resolution_clock::now();

                    // # Frame Tick
                    this->frameTick++;

                    // # Input
                    auto localPlayerInput = cen::PlayerInput{
                        IsKeyDown(KEY_W),
                        IsKeyDown(KEY_S),
                        IsKeyDown(KEY_A),
                        IsKeyDown(KEY_D)
                    };

                    this->playerInputManager.localPlayerInput = localPlayerInput;
                    this->playerInputManager.playerInputs[0] = localPlayerInput;

                    // # Init new nodes
                    this->nodeStorage->InitNewNodes();

                    // # Fixed update
                    auto now = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
                    lastFixedFrameTime = now;
                    accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

                    int fixedUpdateCycles = 0;
                    while (accumulatedFixedTime >= fixedSimulationFrameRateInMs && fixedUpdateCycles < fixedSimulationFrameCyclesLimit) {
                        this->fixedFrameTick++;

                        // # Simulation current Tick
                        this->FixedSimulationTick();

                        // ## Correct time and cycles
                        accumulatedFixedTime -= fixedSimulationFrameRateInMs;
                        fixedUpdateCycles++;
                    }

                    // # Initial
                    for (const auto& node: this->nodeStorage->rootNodes) {
                        node->TraverseUpdate();
                    }

                    // # Flush events
                    this->eventBus.Flush();

                    // # Sync GameState and RendererState
                    auto alpha = static_cast<double>(accumulatedFixedTime.count()) / fixedSimulationFrameRateInMs.count();

                    this->renderingEngine->SyncRenderBuffer(
                        this->nodeStorage.get(),
                        alpha
                    );

                    // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
                    // # End (busy wait)
                    while (std::chrono::high_resolution_clock::now() - frameStart <= simulationFrameRateInMs) {}
                }
            }
    };

    struct SceneChangeRequested: public cen::Event {
        static const std::string type;
        std::string sceneName;

        SceneChangeRequested(): cen::Event(SceneChangeRequested::type) {
            this->sceneName = "";
        }

        SceneChangeRequested(std::string sceneName): cen::Event(SceneChangeRequested::type) {
            this->sceneName = sceneName;
        }
    };

    struct SceneConstructor {
        std::string sceneName;
        std::function<std::unique_ptr<Scene>()> create;

        SceneConstructor(
            std::string sceneName,
            std::function<std::unique_ptr<Scene>()> create
        ): sceneName(sceneName), create(create) {}
    };

    class SceneManager {
        public:
            std::unordered_map<
                scene_name,
                std::unique_ptr<SceneConstructor>
            > scenesConstructorsByName;
            std::unique_ptr<Scene> currentScene;
            std::unique_ptr<Scene> nextScene;
            EventBus* eventBus;
            bool isSimulationRunning = false;

            SceneManager(
                EventBus* eventBus
            ) {
                this->eventBus = eventBus;
                this->currentScene = nullptr;
                this->nextScene = nullptr;

                this->eventBus->On(
                    SceneChangeRequested{},
                    std::make_unique<EventListener>(
                        [this](const Event* event) {
                            auto sceneChangeRequested = static_cast<const SceneChangeRequested*>(event);
                            if (sceneChangeRequested->sceneName == "") {
                                // TODO: SEND ERROR
                                return;
                            }
                            this->ChangeScene(sceneChangeRequested->sceneName);
                        }
                    )
                );
            }

            void AddSceneConstructor(
                std::unique_ptr<SceneConstructor> sceneConstructor
            ) {
                this->scenesConstructorsByName[sceneConstructor->sceneName] = std::move(sceneConstructor);
            }

            void SetFirstScene(scene_name name) {
                if (this->currentScene) {
                    return;
                }

                const auto& constructor = this->scenesConstructorsByName[name];
                this->currentScene = std::move(constructor->create());
            }

            bool ChangeScene(scene_name name) {
                if (this->currentScene == nullptr) {
                    return false;
                }

                const auto& constructor = this->scenesConstructorsByName[name];

                if (constructor == nullptr) {
                    // TODO: SEND ERROR
                    return false;
                }

                this->nextScene = std::move(constructor->create());
                this->StopCurrentSceneSimulation();

                return true;
            }

            bool ChangeScene(std::unique_ptr<Scene> scene) {
                if (this->currentScene == nullptr) {
                    // TODO: SEND ERROR
                    return false;
                }

                if (scene == nullptr) {
                    // TODO: SEND ERROR
                    return false;
                }

                this->nextScene = std::move(scene);
                this->StopCurrentSceneSimulation();

                return true;
            }

            void Run() {
                // # Guard
                if (isSimulationRunning) {
                    return;
                }

                // # Try to assign currentScene if none
                if (this->currentScene == nullptr) {
                    auto it = scenesConstructorsByName.begin();
                    if (it == scenesConstructorsByName.end()) {
                        isSimulationRunning = false;
                        std::cerr << "No scenes to run" << std::endl;
                        return;
                    }
                    const auto& constructor = it->second;
                    this->currentScene = std::move(constructor->create());
                }

                isSimulationRunning = true;

                // # Run simulation
                this->currentScene->Run();

                // # After scene stops
                isSimulationRunning = false;

                // # After it's done
                if (this->nextScene) {
                    this->currentScene = std::move(this->nextScene);
                    this->nextScene = nullptr;
                    this->Run();
                } else {
                    this->currentScene = nullptr;
                }
            }

            void StopCurrentSceneSimulation() {
                if (this->currentScene) {
                    this->currentScene->Stop();
                }
            }

            void Stop() {
                this->nextScene = nullptr;
                this->StopCurrentSceneSimulation();
            }
    };
}

#endif // CENGINE_SCENE_H