#ifndef CENGINE_SCENE_H_
#define CENGINE_SCENE_H_

#include <vector>
#include "node_storage.h"
#include "event.h"

namespace cen {
    class Scene {
        public:
            std::unique_ptr<cen::NodeStorage> node_storage;
            std::vector<std::unique_ptr<cen::TopicBase>> topics;
            EventBus eventBus;

            Scene(
                std::unique_ptr<cen::NodeStorage> node_storage = std::make_unique<NodeStorage>(),
                std::vector<std::unique_ptr<cen::TopicBase>> topics = {}
            ) {
                this->node_storage = std::move(node_storage);
                this->topics = std::move(topics);
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
    };
}

#endif // CENGINE_SCENE_H_