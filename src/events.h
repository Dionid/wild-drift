#include "cengine/cengine.h"

#ifndef CENGINE_EVENTS_H_
#define CENGINE_EVENTS_H_

struct StartEvent: public cen::Event {
    static const std::string type;
    StartEvent(): cen::Event(StartEvent::type) {}
};

#endif // CENGINE_EVENTS_H_