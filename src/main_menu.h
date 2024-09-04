#ifndef CENGINE_MAIN_MENU_H_
#define CENGINE_MAIN_MENU_H_

#include "cengine/node.h"

class MainMenu: public Node {
    public:
        void Render(GameContext* ctx);
        void Init(GameContext* ctx);
};

#endif // CENGINE_MAIN_MENU_H_