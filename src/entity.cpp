#include "entity.h"
#include "utils.h"

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