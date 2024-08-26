#include <vector>
#include <iostream>

class Component;

class GameObject {
    public:
        std::vector<std::shared_ptr<Component>> components;
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

class Component: public Renderer {
    public:
        virtual void Update(GameContext ctx, GameObject* thisGO) {};
};