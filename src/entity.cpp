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

    tileMap->widthInPixels = tileMap->width * tileMap->tileWidth;
    tileMap->heightInPixels = tileMap->height * tileMap->tileHeight;

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

    this->scene->camera->target = Vector2{
        (float)tileMap->widthInPixels / 2,
        (float)tileMap->heightInPixels / 2
    };

    this->scene->camera->offset = Vector2{
        (float)this->scene->screen.width / 2,
        (float)this->scene->screen.height / 2
    };
}

void Map::Update() {
    auto& camera = this->scene->camera;
    const auto& screen = this->scene->screen;

    // # Move by dragging
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera->zoom);

        Vector2 newTarget = Vector2Add(camera->target, delta);

        camera->target = newTarget;
    }

    if (IsKeyDown(KEY_RIGHT)) {
        camera->target.x += 2;
    }

    if (IsKeyDown(KEY_LEFT)) {
        camera->target.x -= 2;
    }

    if (IsKeyDown(KEY_UP)) {
        camera->target.y -= 2;
    }

    if (IsKeyDown(KEY_DOWN)) {
        camera->target.y += 2;
    }

     // # Apply bounds
    if (camera->target.x < camera->offset.x / 2) {
        camera->target.x = camera->offset.x / 2;
    } else if (camera->target.x > tileMap->widthInPixels - camera->offset.x / 2) {
        camera->target.x = tileMap->widthInPixels - camera->offset.x / 2;
    }

    if (camera->target.y < camera->offset.y / 2) {
        camera->target.y = camera->offset.y / 2;
    } else if (camera->target.y > tileMap->heightInPixels - camera->offset.y / 2) {
        camera->target.y = tileMap->heightInPixels - camera->offset.y / 2;
    }
}