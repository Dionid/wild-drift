#ifndef CENGINE_DEBUG_H_
#define CENGINE_DEBUG_H_

#include <raylib.h>
#include "game_context.h"
#include "node.h"
#include "character_body_node_2d.h"

class Debugger: public Renderer {
    public:
        void Render(GameContext* ctx) override {
            DrawText("Debug", 10, 10, 20, WHITE);

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