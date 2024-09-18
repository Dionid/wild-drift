#include "cengine/cengine.h"

#ifndef CENGINE_EVENTS_H_
#define CENGINE_EVENTS_H_

struct StartEvent: public cen::Event {
    static const std::string type;
    StartEvent(): cen::Event(StartEvent::type) {}
};

struct HostEvent: public cen::Event {
    static const std::string type;
    HostEvent(): cen::Event(HostEvent::type) {}
};

struct JoinEvent: public cen::Event {
    static const std::string type;
    JoinEvent(): cen::Event(JoinEvent::type) {}
};

struct RestartEvent: public cen::Event {
    static const std::string type;
    RestartEvent(): cen::Event(RestartEvent::type) {}
};

struct MatchEndEvent: public cen::Event {
    static const std::string type;
    bool isPlayerWon;
    MatchEndEvent(): cen::Event(MatchEndEvent::type) {}
    MatchEndEvent(bool isPlayerWon): cen::Event(MatchEndEvent::type) {
        this->isPlayerWon = isPlayerWon;
    }
};

#endif // CENGINE_EVENTS_H_