#ifndef TIMER_H
#define TIMER_H

#include "node.h"

namespace cen {

class Timer: public Node {
    public:
        float createdAt;
        int duration;

        Timer(
            int duration,
            node_id_t id = 0,
            Node* parent = nullptr
        ): Node(id, parent) {
            this->createdAt = 0;
            this->duration = duration;
        }

        void Init() override {
            this->createdAt = GetTime();
        }

        void Reset() {
            this->createdAt = GetTime();
            this->Activate();
        }

        virtual void OnTimerEnd() = 0;

        void Update() override {
            if (GetTime() - this->createdAt >= this->duration / 1000.0f) {
                this->OnTimerEnd();
                this->Deactivate();
            }
        }
};

} // namespace cen

#endif // TIMER_H