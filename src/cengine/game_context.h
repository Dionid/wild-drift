#ifndef GAME_CONTEXT_H_
#define GAME_CONTEXT_H_

#include <vector>

namespace cen {

class Node;
class Scene;

struct GameContext {
    Scene* scene;
    float worldWidth;
    float worldHeight;
};

}

#endif // GAME_CONTEXT_H_