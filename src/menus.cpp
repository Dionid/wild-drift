#include "cengine/cengine.h"
#include "menus.h"

MainMenu::MainMenu(
    std::function<void()> startCallback
) {
    this->startCallback = startCallback;
}

void MainMenu::Init(GameContext* ctx) {
    this->AddNode(
        std::make_unique<Btn>(
            start,
            btnFontSize,
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
                [this](Btn* btn) {
                    this->startCallback();
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
};

// # MatchEndMenu

void MatchEndMenu::Init(GameContext* ctx) {
    this->AddNode(
        std::make_unique<Btn>(
            "Play again",
            btnFontSize,
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

void MatchEndMenu::Render(GameContext* ctx) {
    DrawText(
        "YOU WIN",
        ctx->worldWidth / 2 - MeasureText("YOU WIN", titleFontSize) / 2,
        ctx->worldHeight / 2 - titleFontSize / 2 - 30,
        titleFontSize,
        WHITE
    );
};