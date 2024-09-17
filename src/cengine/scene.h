#ifndef CENGINE_SCENE_H
#define CENGINE_SCENE_H

#include <vector>
#include <map>
#include <atomic>
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
            EventBus eventBus;

            std::unique_ptr<cen::CollisionEngine> collisionEngine;
            std::unique_ptr<cen::NodeStorage> nodeStorage;
            std::vector<std::unique_ptr<cen::TopicBase>> topics;

            Scene(
                scene_name name,
                cen::ScreenResolution screen,
                Camera2D* camera,
                RenderingEngine2D* renderingEngine,
                EventBus eventBus,
                cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{},
                std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
                std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>(),
                std::vector<std::unique_ptr<cen::TopicBase>> topics = std::vector<std::unique_ptr<cen::TopicBase>>(),
                uint64_t frameTick = 0,
                uint64_t simulationTick = 0
            ) {
                this->name = name;
                this->eventBus = eventBus;
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

                    this->eventBus.flush();

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
        std::string name;

        SceneChangeRequested(): cen::Event(SceneChangeRequested::type) {
            this->name = "";
        }

        SceneChangeRequested(std::string name): cen::Event(SceneChangeRequested::type) {
            this->name = name;
        }
    };

    class SceneManager {
        public:
            std::unordered_map<scene_name, std::unique_ptr<Scene>> scenesByName;
            Scene* currentScene;
            EventBus* eventBus;

            SceneManager(
                EventBus* eventBus
            ) {
                this->eventBus = eventBus;
                this->currentScene = nullptr;

                this->eventBus->on(
                    SceneChangeRequested{},
                    std::make_unique<EventListener>(
                        [this](const Event& event) {
                            std::printf("SceneChangeRequested\n");
                            auto sceneChangeRequested = static_cast<const SceneChangeRequested&>(event);
                            this->SetCurrentScene(sceneChangeRequested.name);
                            this->RunCurrentScene();
                        }
                    )
                );
            }

            cen::Scene* AddScene(std::unique_ptr<Scene> scene) {
                auto scenePtr = scene.get();
                this->scenesByName[scene->name] = std::move(scene);
                return scenePtr;
            }

            void SetCurrentScene(scene_name name) {
                this->currentScene = this->scenesByName[name].get();
            }

            void RunCurrentScene() {
                this->currentScene->RunSimulation();
            }
    };
}

#endif // CENGINE_SCENE_H