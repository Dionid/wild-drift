#ifndef CENGINE_SCENE_H
#define CENGINE_SCENE_H

#include <vector>
#include "rendering.h"
#include "node_storage.h"
#include "collision.h"
#include "event.h"
#include "debug.h"
#include "tick.h"

namespace cen {
    class Scene {
        public:
            u_int64_t frameTick;
            u_int64_t simulationTick;
            Camera2D* camera;
            cen::ScreenResolution screen;
            cen::PlayerInputManager playerInputManager;
            cen::Debugger debugger;
            std::unique_ptr<cen::EventBus> eventBus;
            std::unique_ptr<cen::CollisionEngine> collisionEngine;
            std::unique_ptr<cen::RenderingEngine2D> renderingEngine;
            std::unique_ptr<cen::NodeStorage> nodeStorage;
            std::vector<std::unique_ptr<cen::TopicBase>> topics;

            Scene(
                cen::ScreenResolution screen,
                Camera2D* camera,
                uint64_t frameTick = 0,
                uint64_t simulationTick = 0,
                cen::Debugger debugger = cen::Debugger{},
                std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
                std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>(),
                std::unique_ptr<RenderingEngine2D> renderingEngine = std::make_unique<RenderingEngine2D>(),
                std::vector<std::unique_ptr<cen::TopicBase>> topics = std::vector<std::unique_ptr<cen::TopicBase>>(),
                std::unique_ptr<EventBus> eventBus = std::make_unique<EventBus>(),
                cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{}
            ) {
                this->screen = screen;
                this->camera = camera;
                this->debugger = debugger;
                this->playerInputManager = playerInputManager;
                this->collisionEngine = std::move(collisionEngine);
                this->nodeStorage = std::move(nodeStorage);
                this->nodeStorage->scene = this;
                this->renderingEngine = std::move(renderingEngine);
                this->topics = std::move(topics);
                this->eventBus = std::move(eventBus);
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

            void SimulationTick() {
                // # Game Logic
                // ## Invalidate previous
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseInvalidatePrevious();
                }

                // ## Fixed Update
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseFixedUpdate();
                }

                // ## Collision
                this->collisionEngine->NarrowCollisionCheckNaive(this->nodeStorage.get());
            }

            virtual void Init() {};
    };
}

#endif // CENGINE_SCENE_H