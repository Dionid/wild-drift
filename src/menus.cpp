#include "cengine/cengine.h"
#include "menus.h"
#include "events.h"
#include "globals.h"

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
                    // this->scene->eventBus.Emit(std::make_unique<StartEvent>());
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
                    // this->scene->eventBus.Emit(std::make_unique<HostEvent>());
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
                    // this->scene->eventBus.Emit(std::make_unique<JoinEvent>());
                }
            )
        )
    );
}