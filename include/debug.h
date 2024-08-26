#include <raylib.h>
#include "engine.h"

class Debugger: public Renderer {
    public:
        void Render(GameContext ctx, GameObject* thisGO) override {
            DrawText("Debug", 10, 10, 20, WHITE);
        }
};