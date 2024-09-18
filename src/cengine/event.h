
#ifndef CEN_EVENT_H
#define CEN_EVENT_H

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

    class TopicBase {
        public:
            virtual void flush() = 0;
            virtual void clear() = 0;
    };

    template <typename T>
    class Topic: public TopicBase {
        public:
            std::vector<std::unique_ptr<T>> staged;
            std::vector<std::unique_ptr<T>> ready;

            void emit(
                std::unique_ptr<T> event,
                bool immediate = false
            ) {
                static_assert(std::is_base_of<Event, T>::value, "T must be a subclass of Event");
                if (immediate) {
                    this->ready.push_back(std::move(event));
                } else {
                    this->staged.push_back(std::move(event)); 
                }
            }

            void flush() {
                this->ready.clear();
                for (auto& event : this->staged) {
                    this->ready.push_back(std::move(event));
                }
                this->staged.clear();
            }

            void clear() {
                this->staged.clear();
                this->ready.clear();
            }
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

            int nextId() {
                return this->root->nextEventListenerId.fetch_add(1);
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

#endif // CEN_EVENT_H