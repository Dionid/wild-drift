#include "cengine/cengine.h"
#include "menus.h"

MainMenu::MainMenu(
    std::function<void(cen::GameContext*)> onStart
): cen::Node2D(Vector2{}) {
    this->onStart = onStart;
}

void MainMenu::Init(cen::GameContext* ctx) {
    this->AddNode(
        std::make_unique<cen::Btn>(
            start,
            btnFontSize,
            Vector2{
                ctx->worldWidth / 2,
                ctx->worldHeight / 2 + 50
            },
            cen::Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            cen::Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [this](cen::GameContext* ctx, cen::Btn* btn) {
                    if (this->onStart) {
                        this->onStart(ctx);
                    }
                }
            )
        )
    );

    this->AddNode(
        std::make_unique<cen::TextView>(
            Vector2{
                ctx->worldWidth / 2 - MeasureText(title, titleFontSize) / 2,
                ctx->worldHeight / 2 - titleFontSize / 2 - 30
            },
            title,
            titleFontSize,
            WHITE
        )
    );
}

void MainMenu::Render(cen::GameContext* ctx) {
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
    std::function<void(cen::GameContext*)> onRestart,
    bool playerWon
): cen::Node2D(Vector2{}) {
    this->onRestart = onRestart;
    this->playerWon = playerWon;
}

void MatchEndMenu::Init(cen::GameContext* ctx) {
    this->AddNode(
        std::make_unique<cen::Btn>(
            "Play again",
            btnFontSize,
            Vector2{
                ctx->worldWidth / 2,
                ctx->worldHeight / 2 + 50
            },
            cen::Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            cen::Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [this](cen::GameContext* ctx, cen::Btn* btn) {
                    if (this->onRestart) {
                        this->onRestart(ctx);
                    }
                }
            )
        )
    );

    titleView = this->AddNode(
        std::make_unique<cen::TextView>(
            Vector2{
                ctx->worldWidth / 2 - MeasureText("YOU WON", titleFontSize) / 2,
                ctx->worldHeight / 2 - titleFontSize / 2 - 30
            },
            "YOU WON",
            titleFontSize,
            WHITE
        )
    );
}

void MatchEndMenu::SetPlayerWon(cen::GameContext* ctx, bool playerWon) {
    this->playerWon = playerWon;
    titleView->text = playerWon ? "YOU WON" : "YOU LOST";
    titleView->position = Vector2{
        ctx->worldWidth / 2 - MeasureText(titleView->text.c_str(), titleFontSize) / 2,
        ctx->worldHeight / 2 - titleFontSize / 2 - 30
    };
}

void MatchEndMenu::Render(cen::GameContext* ctx) {
    const char* text = this->playerWon ? "YOU WON" : "YOU LOST";
    DrawText(
        text,
        ctx->worldWidth / 2 - MeasureText(text, titleFontSize) / 2,
        ctx->worldHeight / 2 - titleFontSize / 2 - 30,
        titleFontSize,
        WHITE
    );
};