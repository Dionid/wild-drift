#ifndef CENGINE_DEBUG_H_
#define CENGINE_DEBUG_H_

#include <raylib.h>
#include "game_context.h"
#include "node.h"
#include "character_body_node_2d.h"
#include <unistd.h>

static long get_mem_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

class Debugger: public Renderer {
    public:
        void Render(GameContext* ctx) override {
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

            for (const auto& node: ctx->scene->node_storage->nodes) {
                auto character = dynamic_cast<CharacterBody2D*>(node.get());
                if (character == nullptr) {
                    continue;
                }

                DrawCircleV(character->position, 5, PURPLE);

                DrawLineV(
                    character->position,
                    Vector2Add(character->position, Vector2Scale(character->velocity, 20.f)),
                    GREEN
                );
            }
        }
};

#endif // CENGINE_DEBUG_H_