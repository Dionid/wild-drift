#include "cengine/cengine.h"
#include "menus.h"
#include "events.h"

MainMenu::MainMenu(): cen::Node2D(Vector2{}) {}

void MainMenu::Init() {
    auto yStart = this->scene->screen.height / 2.0f - titleFontSize / 2.0f - 30.0f;
    auto btnStart = yStart + 100.0f;

    this->AddNode(
        std::make_unique<cen::TextView>(
            Vector2{
                this->scene->screen.width / 2.0f - MeasureText(title, titleFontSize) / 2.0f,
                yStart
            },
            title,
            titleFontSize,
            WHITE
        )
    );

    this->AddNode(
        std::make_unique<cen::Btn>(
            start,
            btnFontSize,
            Vector2{
                this->scene->screen.width / 2.0f,
                btnStart
            },
            cen::Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            cen::Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [this](cen::Btn* btn) {
                    this->scene->eventBus.emit(std::make_unique<StartEvent>());
                }
            )
        )
    );

    this->AddNode(
        std::make_unique<cen::Btn>(
            "Host",
            btnFontSize,
            Vector2{
                this->scene->screen.width / 2.0f,
                btnStart + 50.0f
            },
            cen::Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            cen::Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [this](cen::Btn* btn) {
                    this->scene->eventBus.emit(std::make_unique<HostEvent>());
                }
            )
        )
    );

    this->AddNode(
        std::make_unique<cen::Btn>(
            "Join",
            btnFontSize,
            Vector2{
                this->scene->screen.width / 2.0f,
                btnStart + 100.0f
            },
            cen::Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            cen::Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [this](cen::Btn* btn) {
                    this->scene->eventBus.emit(std::make_unique<JoinEvent>());
                }
            )
        )
    );
}

// # MatchEndMenu

MatchEndMenu::MatchEndMenu(
    bool playerWon
): cen::Node2D(Vector2{}) {
    this->playerWon = playerWon;
}

void MatchEndMenu::Init() {
    this->AddNode(
        std::make_unique<cen::Btn>(
            "Play again",
            btnFontSize,
            Vector2{
                this->scene->screen.width / 2.0f,
                this->scene->screen.height / 2.0f + 50.0f
            },
            cen::Size{ 0, 0 },
            Vector2{ 0.5, 0.5 },
            cen::Callbacks(
                nullptr,
                nullptr,
                nullptr,
                [this](cen::Btn* btn) {
                    this->scene->eventBus.emit(std::make_unique<RestartEvent>());
                }
            )
        )
    );

    titleView = this->AddNode(
        std::make_unique<cen::TextView>(
            Vector2{
                this->scene->screen.width / 2.0f - MeasureText("YOU WON", titleFontSize) / 2.0f,
                this->scene->screen.height / 2.0f - titleFontSize / 2.0f - 30.0f
            },
            "YOU WON",
            titleFontSize,
            WHITE
        )
    );
}

void MatchEndMenu::SetPlayerWon(bool playerWon) {
    this->playerWon = playerWon;
    titleView->text = playerWon ? "YOU WON" : "YOU LOST";
    titleView->position = Vector2{
        this->scene->screen.width / 2.0f - MeasureText(titleView->text.c_str(), titleFontSize) / 2.0f,
        this->scene->screen.height / 2.0f - titleFontSize / 2.0f - 30.0f
    };
}