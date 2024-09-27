
#ifndef CENGINE_EVENT_H
#define CENGINE_EVENT_H

#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>

namespace cen {
    struct Event {
        static const std::string type;
        std::string name;

        Event(const std::string& name): name(name) {}
    };

    struct EventListener {
        int id;
        std::function<void(const Event*)> OnEvent;

        EventListener(
            std::function<void(const Event*)> OnEvent,
            int id = 0
        ): id(id), OnEvent(OnEvent) {}
    };

    // TODO: Thread safety
    class EventBus {
        protected:
            void flush() {
                for (size_t i = 0; i < this->events->size(); ++i) {
                    const auto& event = this->events->at(i);
                    for (const auto& listener : this->listeners[event->name]) {
                        listener->OnEvent(event.get());
                    }
                }
            }

            void TraverseFlush() {
                this->flush();
                if (this->parent) {
                    this->parent->TraverseFlush();
                }
            }

            int nextId() {
                return this->root->nextEventListenerId.fetch_add(1);
            }

        public:
            std::shared_ptr<std::vector<std::unique_ptr<Event>>> events;
            std::unordered_map<std::string, std::vector<std::unique_ptr<EventListener>>> listeners;
            EventBus* parent;
            EventBus* root;
            std::atomic<int> nextEventListenerId = 0;

            EventBus(
                EventBus* parent
            ) {
                this->events = parent != nullptr ? parent->events : std::make_shared<std::vector<std::unique_ptr<Event>>>();
                if (parent) {
                    this->root = parent->root;
                } else {
                    this->root = this;
                }
                this->listeners = std::unordered_map<std::string, std::vector<std::unique_ptr<EventListener>>>();
                this->parent = parent;
            }

            EventBus(const EventBus& other) {
                this->parent = other.parent;
                this->nextEventListenerId = 0;
                this->events = other.events;
                this->listeners = std::unordered_map<std::string, std::vector<std::unique_ptr<EventListener>>>();
            }

            EventBus& operator=(const EventBus& other) {
                if (this == &other) {
                    return *this;
                }
                this->parent = other.parent;
                this->nextEventListenerId = 0;
                this->events = other.events;
                this->listeners = std::unordered_map<std::string, std::vector<std::unique_ptr<EventListener>>>();
                return *this;
            }

            ~EventBus() {
                this->listeners.clear();
            }

            int On(
                const Event& event,
                std::unique_ptr<EventListener> listener
            ) {
                int id = listener->id == 0 ? this->nextId() : listener->id;
                listener->id = id;
                this->listeners[event.name].push_back(std::move(listener));

                return id;
            }

            int OnRoot(
                const Event& event,
                std::unique_ptr<EventListener> listener
            ) {
                int id = listener->id == 0 ? this->nextId() : listener->id;
                listener->id = id;
                this->root->listeners[event.name].push_back(std::move(listener));

                return id;
            }

            void Emit(std::unique_ptr<Event> event) {
                this->events->push_back(
                    std::move(event)
                );
            }

            void Off(const Event& event, int listenerId) {
                auto& listenersVec = this->listeners[event.name];
                listenersVec.erase(
                    std::remove_if(
                        listenersVec.begin(),
                        listenersVec.end(),
                        [listenerId](const std::unique_ptr<EventListener>& l) {
                            return l->id == listenerId;
                        }
                    ),
                    listenersVec.end()
                );
            }

            void Flush() {
                this->TraverseFlush();
                this->events->clear();
            }
    };

};

#endif // CENGINE_EVENT_H