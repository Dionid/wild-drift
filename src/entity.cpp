#include <fstream>
#include <nlohmann/json.hpp>
#include "cengine/loader.h"
#include "entity.h"
#include "utils.h"

using json = nlohmann::json;

// # Map

Map::Map(
    std::string name,
    std::string description,
    std::string path,
    Vector2 position,
    int zOrder,
    uint16_t id,
    Node* parent
): Node2D(position, zOrder, id, parent) {
    this->name = name;
    this->description = description;
    this->path = path;
}

void Map::Init() {
    this->AddNode(
        std::make_unique<cen::TextView>(
            Vector2{
                this->scene->screen.width / 2.0f - MeasureText("title", 50.0f) / 2.0f,
                100.0f
            },
            "title",
            50.0f,
            WHITE
        )
    );

    std::ifstream f(cen::GetResourcePath("map/wild-drift-first.json"));
    json data = json::parse(f);

    cen::TileMap tileMap;

    tileMap.path = this->path;
    tileMap.width = data["width"];
    tileMap.height = data["height"];
    tileMap.tileWidth = data["tilewidth"];
    tileMap.tileHeight = data["tileheight"];
    tileMap.orientation = data["orientation"];
    tileMap.renderOrder = data["renderorder"];

    for (auto& tileSet : data["tilesets"]) {
        auto ts = std::make_unique<cen::TileSet>();

        ts->name = tileSet["name"];
        ts->imagePath = tileSet["image"];
        ts->columns = tileSet["columns"];
        ts->tileCount = tileSet["tilecount"];
        ts->tileHeight = tileSet["tileheight"];
        ts->imageHeight = tileSet["imageheight"];
        ts->imageWidth = tileSet["imagewidth"];

        tileMap.tileSets[ts->name] = std::move(ts);
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
        }
    }
}