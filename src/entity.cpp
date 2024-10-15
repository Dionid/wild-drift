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

struct TileSet {
    std::string name;
    std::string imagePath;

    int columns;

    int tileCount;
    int tileHeight;

    int imageHeight;
    int imageWidth;

    Image image;

    ~TileSet() {
        // std::cout << "TileSet destructor" << std::endl;
        UnloadImage(this->image);
    }
};

struct TileMap {
    std::string path;

    int width;
    int height;

    int tileWidth;
    int tileHeight;

    std::string orientation;
    std::string renderOrder;

    std::vector<std::unique_ptr<TileSet>> tileSets;
};

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

    TileMap tileMap;

    tileMap.path = this->path;
    tileMap.width = data["width"];
    tileMap.height = data["height"];
    tileMap.tileWidth = data["tilewidth"];
    tileMap.tileHeight = data["tileheight"];
    tileMap.orientation = data["orientation"];
    tileMap.renderOrder = data["renderorder"];

    for (auto& tileSet : data["tilesets"]) {
        auto ts = std::make_unique<TileSet>();

        ts->name = tileSet["name"];
        ts->imagePath = tileSet["image"];
        ts->columns = tileSet["columns"];
        ts->tileCount = tileSet["tilecount"];
        ts->tileHeight = tileSet["tileheight"];
        ts->imageHeight = tileSet["imageheight"];
        ts->imageWidth = tileSet["imagewidth"];

        ts->image = LoadImage(cen::GetResourcePath("map/" + ts->imagePath).c_str());

        tileMap.tileSets.push_back(std::move(ts));
    }
}