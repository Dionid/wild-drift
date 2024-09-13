#ifndef GAME_CONTEXT_H_
#define GAME_CONTEXT_H_

#include <vector>
#include "core.h"

namespace cen {

class Node;
class Scene;

struct GameContext {
    Scene* scene;
    PlayerInput& playerInput;
    float worldWidth;
    float worldHeight;
};

}

#endif // GAME_CONTEXT_H_