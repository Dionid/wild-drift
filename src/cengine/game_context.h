#ifndef GAME_CONTEXT_H_
#define GAME_CONTEXT_H_

#include <vector>

class Node;
class CollisionEngine;

namespace cen {

class Scene;

struct GameContext {
    Scene* scene;
    CollisionEngine* collisionEngine;
    float worldWidth;
    float worldHeight;
};

}

#endif // GAME_CONTEXT_H_