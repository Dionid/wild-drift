#include "cengine/cengine.h"
#include "menus.h"

MainMenu::MainMenu(
    std::function<void(GameContext*)> onStart
) {
    this->onStart = onStart;
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
                [this](GameContext* ctx, Btn* btn) {
                    if (this->onStart) {
                        this->onStart(ctx);
                    }
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

MatchEndMenu::MatchEndMenu(
    std::function<void(GameContext*)> onRestart,
    bool playerWon
) {
    this->onRestart = onRestart;
    this->playerWon = playerWon;
}

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
                [this](GameContext* ctx, Btn* btn) {
                    if (this->onRestart) {
                        this->onRestart(ctx);
                    }
                }
            )
        )
    );
}

void MatchEndMenu::Render(GameContext* ctx) {
    const char* text = this->playerWon ? "YOU WON" : "YOU LOST";
    DrawText(
        text,
        ctx->worldWidth / 2 - MeasureText(text, titleFontSize) / 2,
        ctx->worldHeight / 2 - titleFontSize / 2 - 30,
        titleFontSize,
        WHITE
    );
};