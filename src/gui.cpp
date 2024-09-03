#include "gui.h"

void MainMenu::Render(GameContext* ctx) {
    int fontSize = 50;
    DrawText(
        "Super Pong",
        ctx->worldWidth / 2 - fontSize / 2,
        ctx->worldHeight / 2 - fontSize / 2,
        fontSize,
        WHITE
    );
};