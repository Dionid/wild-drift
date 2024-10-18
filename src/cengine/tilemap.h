#ifndef CEN_TILEMAP_H
#define CEN_TILEMAP_H

#include "core.h"

using json = nlohmann::json;

namespace cen {
    struct TileSet {
        std::string name;
        std::string imagePath;

        int columns;

        int tileCount;
        int tileHeight;

        int imageHeight;
        int imageWidth;

        int firstGID;
        int lastGID;
    };

    struct TileMapLayer {
        int id;
        std::string name;

        int width;
        int height;
        bool visible;
        float opacity;
        bool ySort;

        std::string type;
        std::vector<int> data;
    };

    struct TileMap {
        std::string path;

        int width;
        int height;

        int widthInPixels;
        int heightInPixels;

        int tileWidth;
        int tileHeight;

        std::string orientation;
        std::string renderOrder;

        std::vector<TileMapLayer> layers;
        std::unordered_map<std::string, TileMapLayer*> layersByName;
        std::unordered_map<std::string, std::unique_ptr<TileSet>> tileSets;

        TileMap(
            std::string path
        ) {
            this->path = path;
        }

        void Load() {
            std::string jsonPath = "map/" + this->path + ".json";
            std::ifstream f(cen::GetResourcePath(jsonPath));
            json data = json::parse(f);

            this->width = data["width"];
            this->height = data["height"];
            this->tileWidth = data["tilewidth"];
            this->tileHeight = data["tileheight"];
            this->orientation = data["orientation"];
            this->renderOrder = data["renderorder"];

            this->widthInPixels = this->width * this->tileWidth;
            this->heightInPixels = this->height * this->tileHeight;

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

                this->tileSets[ts->name] = std::move(ts);
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
                    auto properties = layer["properties"];

                    if (properties != nullptr) {
                        if (properties.contains("ySort")) {
                            tileMapLayer.ySort = properties["ySort"];
                        }
                    }

                    for (auto& tile : layer["data"]) {
                        tileMapLayer.data.push_back(tile);
                    }

                    this->layers.push_back(tileMapLayer);
                    this->layersByName[tileMapLayer.name] = &this->layers.back();
                }
            }
        }
    };
}

#endif // CEN_TILEMAP_H