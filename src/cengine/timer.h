#ifndef CENGINE_TIMER_H
#define CENGINE_TIMER_H

#include "node.h"

namespace cen {

enum class TimerMode {
    MILLISECONDS,
    FRAMES,
    FIXED_FRAMES
};

class Timer: public Node {
    public:
        float createdAt;
        int triggerAfter;
        TimerMode mode;

        Timer(
            int triggerAfter,
            TimerMode mode,
            node_id_t id = 0,
            Node* parent = nullptr
        ): Node(id, parent) {
            this->createdAt = 0;
            this->mode = mode;
            this->triggerAfter = triggerAfter;
        }

        void SetCreatedAt() {
            switch(this->mode) {
                case TimerMode::MILLISECONDS:
                    this->createdAt = GetTime();
                    break;
                case TimerMode::FRAMES:
                    this->createdAt = this->scene->frameTick;
                    break;
                case TimerMode::FIXED_FRAMES:
                    this->createdAt = this->scene->fixedFrameTick;
                    break;
            }
        }

        void Init() override {
            this->SetCreatedAt();
        }

        void Reset() {
            this->SetCreatedAt();
            this->Activate();
        }

        virtual void OnTimerEnd() = 0;

        void Update() override {
            switch(this->mode) {
                case TimerMode::MILLISECONDS:
                    if (GetTime() - this->createdAt >= this->triggerAfter / 1000.0f) {
                        this->OnTimerEnd();
                        this->Deactivate();
                    }
                    break;
                case TimerMode::FRAMES:
                    if (this->scene->frameTick - this->createdAt >= this->triggerAfter) {
                        this->OnTimerEnd();
                        this->Deactivate();
                    }
                    break;
                case TimerMode::FIXED_FRAMES:
                    if (this->scene->fixedFrameTick - this->createdAt >= this->triggerAfter) {
                        this->OnTimerEnd();
                        this->Deactivate();
                    }
                    break;
                default:
                    std::cerr << "Timer::Update: Unknown mode" << std::endl;
                    break;
            }
        }
};

} // namespace cen

#endif // CENGINE_TIMER_H