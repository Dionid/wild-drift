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
            u_int64_t fixedSimulationTick;

            Camera2D* camera;
            cen::ScreenResolution screen;
            cen::PlayerInputManager playerInputManager;
            cen::RenderingEngine2D* renderingEngine;
            cen::EventBus eventBus;

            std::unique_ptr<cen::CollisionEngine> collisionEngine;
            std::unique_ptr<cen::NodeStorage> nodeStorage;
            std::vector<std::unique_ptr<cen::TopicBase>> topics;

            Scene(
                scene_name name,
                cen::ScreenResolution screen,
                Camera2D* camera,
                RenderingEngine2D* renderingEngine,
                cen::EventBus* eventBus,
                cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{},
                std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
                std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>(),
                std::vector<std::unique_ptr<cen::TopicBase>> topics = std::vector<std::unique_ptr<cen::TopicBase>>(),
                uint64_t frameTick = 0,
                uint64_t simulationTick = 0
            ): eventBus(eventBus) {
                this->name = name;
                // this->eventBus = cen::EventBus(eventBus);
                this->screen = screen;
                this->camera = camera;
                this->renderingEngine = renderingEngine;
                this->playerInputManager = playerInputManager;
                this->collisionEngine = std::move(collisionEngine);
                this->nodeStorage = std::move(nodeStorage);
                this->nodeStorage->scene = this;
                this->topics = std::move(topics);
            }

            virtual void Init() {};
            virtual void Destroy() {};

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

            template <typename T>
            cen::Topic<T>* AddTopic(std::unique_ptr<cen::Topic<T>> topic) {
                static_assert(std::is_base_of<cen::TopicBase, cen::Topic<T>>::value, "T must inherit from TopicBase");

                auto topicPtr = topic.get();

                this->topics.push_back(
                    std::move(topic)
                );

                return topicPtr;
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

            void RunSimulation() {
                if (!this->isInitialized) {
                    // # Init scene
                    this->FullInit();
                }

                // # Main Loop
                // ## Update
                const int targetFPS = 60;
                const std::chrono::milliseconds targetFrameTime(1000 / targetFPS);

                // ## Fixed update
                const int fixedUpdateRate = 40;
                const std::chrono::milliseconds targetFixedUpdateTime(1000 / fixedUpdateRate);
                int fixedUpdateCyclesLimit = 10;
                std::chrono::milliseconds accumulatedFixedTime(0);
                auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();

                while (this->isAlive.load(std::memory_order_acquire))    // Detect window close button or ESC key
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

                    // # Init new nodes
                    this->nodeStorage->InitNewNodes();

                    // # Fixed update
                    auto now = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
                    lastFixedFrameTime = now;
                    accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

                    int fixedUpdateCycles = 0;
                    while (accumulatedFixedTime >= targetFixedUpdateTime && fixedUpdateCycles < fixedUpdateCyclesLimit) {
                        this->fixedSimulationTick++;

                        // # Simulation current Tick
                        this->FixedSimulationTick();

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

                    this->eventBus.Flush();

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

            void RunSceneSimulation() {
                // # Guard
                if (isSimulationRunning) {
                    return;
                }

                // # Try to assign currentScene if none
                if (this->currentScene == nullptr) {
                    auto it = scenesConstructorsByName.begin();
                    if (it == scenesConstructorsByName.end()) {
                        isSimulationRunning = false;
                        std::cout << "No scenes to run" << std::endl;
                        return;
                    }
                    const auto& constructor = it->second;
                    this->currentScene = std::move(constructor->create());
                }

                isSimulationRunning = true;

                // # Run simulation
                this->currentScene->RunSimulation();

                isSimulationRunning = false;

                // # After it's done
                this->currentScene->Destroy();
                if (this->nextScene) {
                    this->currentScene = std::move(this->nextScene);
                    this->nextScene = nullptr;
                    this->RunSceneSimulation();
                } else {
                    this->currentScene = nullptr;
                }
            }

            void StopCurrentSceneSimulation() {
                if (this->currentScene) {
                    this->currentScene->isAlive.store(false, std::memory_order_release);
                }
            }

            void StopTheWorld() {
                this->nextScene = nullptr;
                this->StopCurrentSceneSimulation();
                // ...
            }
    };
}

#endif // CENGINE_SCENE_H