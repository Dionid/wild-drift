#ifndef CEN_LOADER_H_
#define CEN_LOADER_H_

#include <string>
#include "mac.h"

#ifndef ASSETS_PATH
#define ASSETS_PATH "/assets/"
#endif

namespace cen {

static std::string GetResourcePath(const std::string& resource) {
    if (__APPLE__) {
        return cen::macutils::GetResourcePath(resource);
    } else {
        return ASSETS_PATH + resource;
    }
}

}

#endif // CEN_LOADER_H_