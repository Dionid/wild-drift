#include "scenes.h"
#include "entity.h"

// # MainMenuScene

void MainMenuScene::Init() {
    EnableCursor();

    // ## MainMenu
    MainMenu* mainMenu = this->AddNode(std::make_unique<MainMenu>());

    this->eventBus.On(
        StartEvent{},
        std::make_unique<cen::EventListener>(
            [
                this
            ](const cen::Event* event) {
                this->eventBus.Emit(
                    std::make_unique<cen::SceneChangeRequested>(
                        LocalMatchSceneName
                    )
                );
            }
        )
    );
}

// # LocalMatchScene

void LocalMatchScene::Init() {
    DisableCursor();

    this->AddNode(
        std::make_unique<Map>(
            "main",
            "main map",
            "",
            Vector2{ 0, 0 }
        )
    );

    auto yStart = this->screen.height / 2.0f - 50.0f / 2.0f - 30.0f;
    const char* title = "MATCH";

    this->AddNode(
        std::make_unique<cen::TextView>(
            Vector2{
                this->screen.width / 2.0f - MeasureText(title, 50.0f) / 2.0f,
                yStart
            },
            title,
            50.0f,
            WHITE
        )
    );
}