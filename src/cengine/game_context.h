#ifndef GAME_CONTEXT_H_
#define GAME_CONTEXT_H_

#include <vector>

namespace cen {

class Node;
class Scene;
class PlayerInput;

struct GameContext {
    Scene* scene;
    PlayerInput& playerInput;
    float worldWidth;
    float worldHeight;
};

}

#endif // GAME_CONTEXT_H_