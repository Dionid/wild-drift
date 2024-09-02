#include <vector>

#ifndef GAME_CONTEXT_H_
#define GAME_CONTEXT_H_

// # GameContext

class Node;

struct GameContext {
    std::vector<std::shared_ptr<Node>> nodes;
    float worldWidth;
    float worldHeight;
};

#endif // GAME_CONTEXT_H_