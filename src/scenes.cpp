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

    this->camera->zoom = 2.0f;

    auto map = this->AddNode(
        std::make_unique<Map>(
            "main",
            "wild-drift-first",
            textureStorage,
            Vector2{ 0, 0 }
        )
    );

    auto player = this->AddNode(
        std::make_unique<Player>(
            0,
            "Player 1",
            Vector2{
                (float)map->tileMap->widthInPixels / 2,
                (float)map->tileMap->heightInPixels / 2
            },
            cen::Size{ 32, 32 },
            2.0f,
            4.0f
        )
    );

    this->AddNode(
        std::make_unique<WDCamera>(
            map,
            player
        )
    );
}

void LocalMatchScene::BeforeStop() {
    EnableCursor();
    this->camera->zoom = 1.0f;
}