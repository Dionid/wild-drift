#ifndef GAME_CONTEXT_H_
#define GAME_CONTEXT_H_

#include <vector>

// # GameContext

class Node;
class Scene;
class CollisionEngine;

struct GameContext {
    Scene* scene;
    CollisionEngine* collisionEngine;
    float worldWidth;
    float worldHeight;
};

#endif // GAME_CONTEXT_H_