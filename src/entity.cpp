#include <fstream>
#include <nlohmann/json.hpp>
#include "entity.h"
#include "utils.h"

using json = nlohmann::json;

// # Map

Map::Map(
    std::string name,
    std::string description,
    std::string path,
    cen::TextureStorage* textureStorage,
    Vector2 position,
    int zOrder,
    uint16_t id,
    Node* parent
): Node2D(position, zOrder, id, parent) {
    this->name = name;
    this->description = description;
    this->path = path;
    this->textureStorage = textureStorage;
}

void Map::Init() {
    std::ifstream f(cen::GetResourcePath("map/wild-drift-first.json"));
    json data = json::parse(f);

    this->tileMap = std::make_unique<cen::TileMap>();

    tileMap->path = this->path;
    tileMap->width = data["width"];
    tileMap->height = data["height"];
    tileMap->tileWidth = data["tilewidth"];
    tileMap->tileHeight = data["tileheight"];
    tileMap->orientation = data["orientation"];
    tileMap->renderOrder = data["renderorder"];

    for (auto& tileSet : data["tilesets"]) {
        auto ts = std::make_unique<cen::TileSet>();

        ts->name = tileSet["name"];
        ts->imagePath = tileSet["image"];
        ts->columns = tileSet["columns"];
        ts->tileCount = tileSet["tilecount"];
        ts->tileHeight = tileSet["tileheight"];
        ts->imageHeight = tileSet["imageheight"];
        ts->imageWidth = tileSet["imagewidth"];
        ts->firstGID = tileSet["firstgid"];
        ts->lastGID = ts->firstGID + ts->tileCount - 1;

        tileMap->tileSets[ts->name] = std::move(ts);
    }

    for (auto& layer : data["layers"]) {
        if (layer["type"] == "tilelayer") {
            cen::TileMapLayer tileMapLayer;

            tileMapLayer.id = layer["id"];
            tileMapLayer.name = layer["name"];
            tileMapLayer.width = layer["width"];
            tileMapLayer.height = layer["height"];
            tileMapLayer.visible = layer["visible"];
            tileMapLayer.type = layer["type"];
            tileMapLayer.opacity = layer["opacity"];

            for (auto& tile : layer["data"]) {
                tileMapLayer.data.push_back(tile);
            }

            tileMap->layers.push_back(tileMapLayer);
        }
    }

    this->AddNode(
        std::make_unique<cen::TileMapView>(
            tileMap.get(),
            this->textureStorage
        )
    );
}

void Map::Update() {
    // # Move by dragging
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        auto& camera = this->scene->camera;
        const auto& screen = this->scene->screen;

        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera->zoom);

        Vector2 newTarget = Vector2Add(camera->target, delta);

        if (newTarget.x < 0) {
            newTarget.x = 0;
        }

        if (newTarget.y < 0) {
            newTarget.y = 0;
        }

        // # check if it is more than screen with zoom
        if (newTarget.x > tileMap->width * tileMap->tileWidth - screen.width / camera->zoom) {
            newTarget.x = tileMap->width * tileMap->tileWidth - screen.width / camera->zoom;
        }

        if (newTarget.y > tileMap->height * tileMap->tileHeight - screen.height / camera->zoom) {
            newTarget.y = tileMap->height * tileMap->tileHeight - screen.height / camera->zoom;
        }

        camera->target = newTarget;
    }
}