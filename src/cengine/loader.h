#include <string>
#include "mac.h"

#ifndef ASSETS_PATH
#define ASSETS_PATH "/assets/"
#endif

std::string GetResourcePath(const std::string& resource) {
    if (__APPLE__) {
        return macutils::GetResourcePath(resource);
    } else {
        return ASSETS_PATH + resource;
    }
}

