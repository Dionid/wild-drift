#ifndef CENGINE_DEBUG_H_
#define CENGINE_DEBUG_H_

#include <raylib.h>
#include <unistd.h>

namespace cen {

#ifndef CEN_DEBUG
#define CEN_DEBUG = 0
#endif // CEN_DEBUG

static long get_mem_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

class Debugger {
    public:
        void DrawDebugInfo() {
            DrawText(
                TextFormat(
                    "Debug mode (%d)", getpid()
                ),
                10,
                10,
                15,
                WHITE
            );
            DrawText(
                TextFormat("Mem: %.2f MB", get_mem_usage() / 1000000.0f),
                10,
                25,
                15,
                WHITE
            );
            DrawText(
                TextFormat("FPS: %d", GetFPS()),
                10,
                40,
                15,
                WHITE
            );
        }

        void Render() {
            this->DrawDebugInfo();
        }
};

}

#endif // CENGINE_DEBUG_H_