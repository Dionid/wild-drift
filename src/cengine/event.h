
#ifndef CEN_EVENT_H
#define CEN_EVENT_H

#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "game_context.h"

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

    #define EventListener std::function<void(cen::GameContext*, const Event&)>

    class EventBus {
        public:
            std::unordered_map<std::string, std::vector<EventListener>> listeners;
            std::vector<Event> events;

            void on(
                const Event& event,
                EventListener listener
            ) {
                this->listeners[event.name].push_back(listener);
            }

            void emit(const Event& event) {
                this->events.push_back(event);
            }

            // void off(const Event& event, EventListener listener) {
            //     auto& listenersVec = this->listeners[event.name];
            //     listenersVec.erase(
            //         std::remove(listenersVec.begin(), listenersVec.end(), listener),
            //         listenersVec.end()
            //     );
            // }

            void flush(cen::GameContext* ctx) {
                for (auto& event : this->events) {
                    auto listenersVec = this->listeners[event.name];
                    for (auto& listener : listenersVec) {
                        listener(ctx, event);
                    }
                }
                this->events.clear();
            }
    };

};

#endif // CEN_EVENT_H