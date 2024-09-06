#ifndef SPC_AUDIO_H_
#define SPC_AUDIO_H_

#include <memory>
#include <raylib.h>

struct SpcAudio {
    Sound start;
    Sound hit;
    Sound score;
    Sound lost;
    Sound win;
};

#endif // SPC_AUDIO_H_