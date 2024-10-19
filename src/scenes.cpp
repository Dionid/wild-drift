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

    for (int layerIndex = 0; layerIndex < map->tileMap->layers.size(); ++layerIndex) {
        auto& layer = map->tileMap->layers[layerIndex];
        if (layer.solid == true || layer.sensor == true) {
            for (int y = 0; y < map->tileMap->height; ++y) {
                for (int x = 0; x < map->tileMap->width; ++x) {
                    int tileGID = layer.data[y * map->tileMap->width + x];

                    if (tileGID <= 0) {
                        continue;
                    }

                    this->AddNode(
                        std::make_unique<cen::RectangleCollisionObject2D>(
                            Vector2{
                                (float)x * map->tileMap->tileWidth + (float)map->tileMap->tileWidth / 2,
                                (float)y * map->tileMap->tileHeight + (float)map->tileMap->tileHeight / 2
                            },
                            layer.solid ? cen::ColliderType::Solid : cen::ColliderType::Sensor,
                            cen::Size{
                                (float)map->tileMap->tileWidth,
                                (float)map->tileMap->tileHeight
                            }
                        )
                    );
                }
            }
        }
    }

    auto player = this->AddNode(
        std::make_unique<Player>(
            0,
            "Player 1",
            Vector2{
                (float)map->width / 2,
                (float)map->height / 2
            },
            cen::Size{ 32, 32 },
            2.0f,
            4.0f,
            map
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