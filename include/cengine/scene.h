#ifndef CENGINE_SCENE_H_
#define CENGINE_SCENE_H_

#include <vector>
#include "node_storage.h"

class Scene {
    public:
        std::unique_ptr<NodeStorage> node_storage;

        Scene(
            std::unique_ptr<NodeStorage> node_storage = std::make_unique<NodeStorage>()
        ) {
            this->node_storage = std::move(node_storage);
        }
};

#endif // CENGINE_SCENE_H_