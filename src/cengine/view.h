#ifndef CENGINE_VIEW_H_
#define CENGINE_VIEW_H_

#include "texture.h"
#include "tilemap.h"
#include "node_2d.h"

// # Views

namespace cen {

class TextView: public cen::Node2D {
    public:
        std::string  text;
        int fontSize;
        Color color;

        TextView(
            Vector2 position,
            std::string text,
            int fontSize,
            Color color
        ): cen::Node2D(position) {
            this->text = text;
            this->fontSize = fontSize;
            this->color = color;
        }
};

class LineView: public cen::Node2D {
    public:
        float length;
        float alpha;
        Color color;

        LineView(Vector2 position, float length, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, zOrder, id, parent) {
            this->length = length;
            this->alpha = alpha;
            this->color = color;
        }
};

class CircleView: public cen::Node2D {
    public:
        float radius;
        float alpha;
        Color color;
        bool fill;

        CircleView(float radius, Vector2 position = Vector2{}, Color color = WHITE, float alpha = 1.0f, bool fill = true, int zOrder = 0,  Vector2 globalPosition = Vector2{}, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, zOrder, id, parent) {
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }
};

class RectangleView: public cen::Node2D {
    public:
        cen::Size size;
        Color color;
        float alpha;

        RectangleView(cen::Size size, Color color = WHITE, float alpha = 1.0f, Vector2 position = Vector2{}, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, zOrder, id, parent) {
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }
};


class TileView: public cen::Node2D {
    public:
        TileMap* map;
        Size size;
        Rectangle texturePosition;
        Texture texture;

        TileView(
            TileMap* map,
            Texture texture,
            Rectangle texturePosition,
            Vector2 position,
            Size size,
            int zOrder = 0,
            uint16_t id = 0,
            Node* parent = nullptr
        ): cen::Node2D(position, zOrder, id, parent) {
            this->map = map;
            this->size = size;
            this->texturePosition = texturePosition;
            this->texture = texture;
        }
};

class TileMapView: public cen::Node2D {
    public:
        TileMap* map;
        TextureStorage* textureStorage;

        TileMapView(
            TileMap* map,
            TextureStorage* textureStorage,
            Vector2 position = Vector2{},
            int zOrder = 0,
            uint16_t id = 0,
            Node* parent = nullptr
        ): cen::Node2D(position, zOrder, id, parent) {
            this->map = map;
            this->textureStorage = textureStorage;
        }

        void Init() override {
            printf("TileMapView %d %d %zu\n", map->height, map->width, map->layers.size());

            for (const auto& layer: map->layers) {
                std::cout << "Layer: " << layer.name << std::endl;

                for (int y = 0; y < map->height; ++y) {
                    for (int x = 0; x < map->width; ++x) {
                        int tileGID = layer.data[y * map->width + x];

                        if (tileGID <= 0) {
                            continue;
                        }

                        for (const auto& [name, tileSet]: map->tileSets) {
                            if (tileGID >= tileSet->firstGID && tileGID <= tileSet->lastGID) {
                                int tileX = (tileGID - tileSet->firstGID) % tileSet->columns;
                                int tileY = (tileGID - tileSet->firstGID) / tileSet->columns;

                                std::cout << "Tile: " << tileX << ", " << tileY << std::endl;

                                this->AddNode(
                                    std::make_unique<TileView>(
                                        map,
                                        *this->textureStorage->data[tileSet->name],
                                        Rectangle{
                                            (float)tileX * map->tileWidth,
                                            (float)tileY * map->tileHeight,
                                            (float)map->tileWidth,
                                            (float)map->tileHeight
                                        },
                                        Vector2{
                                            (float)x * map->tileWidth,
                                            (float)y * map->tileHeight
                                        },
                                        Size{
                                            (float)map->tileWidth,
                                            (float)map->tileHeight
                                        }
                                    )
                                );
                            }
                        }
                    }
                }
            }
        }
};

} // namespace cen

#endif // CENGINE_VIEW_H_