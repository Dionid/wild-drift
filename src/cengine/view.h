#ifndef CENGINE_VIEW_H_
#define CENGINE_VIEW_H_

#include "texture.h"
#include "tilemap.h"
#include "node_2d.h"
#include "collision.h"

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

        LineView(Vector2 position, float length, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, bool ySort = false, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, ySort, zOrder, id, parent) {
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

        CircleView(float radius, Vector2 position = Vector2{}, Color color = WHITE, float alpha = 1.0f, bool fill = true, bool ySort = false, int zOrder = 0,  Vector2 globalPosition = Vector2{}, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, ySort, zOrder, id, parent) {
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

        RectangleView(cen::Size size, Color color = WHITE, float alpha = 1.0f, Vector2 position = Vector2{}, bool ySort = false, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, ySort, zOrder, id, parent) {
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }
};

class TextureView: public cen::Node2D {
    public:
        Texture texture;
        Rectangle texturePosition;
        Size size;

        TextureView(
            Texture texture,
            Rectangle texturePosition,
            Vector2 position,
            Size size,
            bool ySort = false,
            int zOrder = 0,
            uint16_t id = 0,
            Node* parent = nullptr
        ): cen::Node2D(position, ySort, zOrder, id, parent) {
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
            bool ySort = false,
            int zOrder = 0,
            uint16_t id = 0,
            Node* parent = nullptr
        ): cen::Node2D(position, ySort, zOrder, id, parent) {
            this->map = map;
            this->textureStorage = textureStorage;
        }

        void Init() override {
            for (int layerIndex = 0; layerIndex < map->layers.size(); ++layerIndex) {
                auto& layer = map->layers[layerIndex];
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

                                this->AddNode(
                                    std::make_unique<TextureView>(
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
                                        },
                                        layer.ySort,
                                        layer.ySort ? (y + 1) * map->tileHeight : 0
                                    )
                                );

                                // if (layer.solid || layer.sensor) {
                                //     std::cout << "Adding collision object" << std::endl;
                                //     this->AddNode(
                                //         std::make_unique<RectangleCollisionObject2D>(
                                //             Vector2{
                                //                 (float)x * map->tileWidth,
                                //                 (float)y * map->tileHeight
                                //             },
                                //             layer.solid ? ColliderType::Solid : ColliderType::Sensor,
                                //             Size{
                                //                 (float)map->tileWidth,
                                //                 (float)map->tileHeight
                                //             }
                                //         )
                                //     );
                                // }
                            }
                        }
                    }
                }
            }
        }
};

} // namespace cen

#endif // CENGINE_VIEW_H_