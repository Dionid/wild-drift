#ifndef CEN_TEXTURE_H
#define CEN_TEXTURE_H

#include <unordered_map>
#include "core.h"
#include "loader.h"

namespace cen {
    struct TextureStorage {
        std::string prefix;
        std::unordered_map<
            std::string,
            std::unique_ptr<Texture>
        > data;

        ~TextureStorage() {
            for (auto& [key, value] : data) {
                UnloadTexture(*value);
            }
        }

        void AddTexture(const std::string& key, const std::string& path) {
            data[key] = std::make_unique<Texture>(
                LoadTexture(
                    cen::GetResourcePath(this->prefix + path).c_str()
                )
            );
        }
    };
}

#endif // CEN_TEXTURE_H