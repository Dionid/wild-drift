#ifndef CEN_TILEMAP_H
#define CEN_TILEMAP_H

#include "core.h"

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

        std::string type;
        std::vector<int> data;
    };

    struct TileMap {
        std::string path;

        int width;
        int height;

        int tileWidth;
        int tileHeight;

        std::string orientation;
        std::string renderOrder;

        std::vector<TileMapLayer> layers;
        std::unordered_map<std::string, std::unique_ptr<TileSet>> tileSets;
    };
}

#endif // CEN_TILEMAP_H