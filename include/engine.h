#include <vector>
#include <iostream>

// # Core

class Node;

class GameObject {
    public:
        std::shared_ptr<Node> rootNode;
};

struct GameContext {
    std::vector<GameObject*> gos;
    float worldWidth;
    float worldHeight;
};

class Renderer {
    public:
        virtual void Render(GameContext ctx, GameObject* thisGO) {};
};

class Updater {
    public:
        virtual void Update(GameContext ctx, GameObject* thisGO) {};
};

class Node: public Renderer, public Updater {
    public:
        std::vector<std::shared_ptr<Node>> nodes;
};

// # Types

struct Size {
    float width;
    float height;
};

// # Delta
const float FPS = 60.0f;
const float secondsPerFrame = 1.0f / FPS;

float DeltaTime() {
    return GetFrameTime() / secondsPerFrame;
}