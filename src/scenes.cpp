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
            textureStorage,
            Vector2{ 0, 0 }
        )
    );
}