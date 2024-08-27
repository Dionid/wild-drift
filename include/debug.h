#include <raylib.h>
#include "engine.h"
#include "character-body-node-2d.h"

#ifndef CDEBUG_H
#define CDEBUG_H

class Debugger: public Renderer {
    public:
        void Render(GameContext* ctx) override {
            DrawText("Debug", 10, 10, 20, WHITE);

            for (auto node: ctx->nodes) {
                auto character = std::dynamic_pointer_cast<CharacterBody2D>(node);
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

#endif // CDEBUG_H