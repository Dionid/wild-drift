#include "cengine/cengine.h"
#include "main_menu.h"

int titleFontSize = 50;
const char* title = "Super Pong";

int startFontSize = 20;
const char* start = "Start";

void MainMenu::Init(GameContext* ctx) {
    this->AddNode(
        std::make_unique<Btn>(
            start,
            startFontSize,
            Vector2{
                ctx->worldWidth / 2,
                ctx->worldHeight / 2 + 50
            },
            Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [](Btn* btn) {
                    printf("Button clicked!\n");
                }
            )
        )
    );
}

void MainMenu::Render(GameContext* ctx) {
    DrawText(
        title,
        ctx->worldWidth / 2 - MeasureText(title, titleFontSize) / 2,
        ctx->worldHeight / 2 - titleFontSize / 2 - 30,
        titleFontSize,
        WHITE
    );

    // Draw button
    // Btn btn = Btn(
    //     start,
    //     startFontSize,
    //     Vector2{
    //         ctx->worldWidth / 2,
    //         ctx->worldHeight / 2 + 50
    //     },
    //     Size{ 0, 0 },
    //     Vector2{ 0.5, 0.5 },
    //     Callbacks(
    //         nullptr,
    //         nullptr,
    //         nullptr,
    //         [](Btn* btn) {
    //             printf("Button clicked!\n");
    //         }
    //     )
    // );
    // btn.Render(ctx);
};