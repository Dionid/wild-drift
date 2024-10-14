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
}