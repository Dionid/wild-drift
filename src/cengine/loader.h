#include <string>
#include "mac.h"

#ifndef ASSETS_PATH
#define ASSETS_PATH "/assets/"
#endif

namespace cen {

std::string GetResourcePath(const std::string& resource) {
    if (__APPLE__) {
        return cen::macutils::GetResourcePath(resource);
    } else {
        return ASSETS_PATH + resource;
    }
}

}