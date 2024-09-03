
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "gui.h"

int fontSize = 50;
const char* title = "Super Pong";
const char* start = "Start";

void MainMenu::Render(GameContext* ctx) {
    DrawText(
        title,
        ctx->worldWidth / 2 - MeasureText(title, fontSize) / 2,
        ctx->worldHeight / 2 - fontSize / 2,
        fontSize,
        WHITE
    );

    GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message");
};