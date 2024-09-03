
#ifndef CENGINE_GUI_H_
#define CENGINE_GUI_H_

#include "cengine/node.h"

class MainMenu: public Node {
    public:
        void Render(GameContext* ctx);
};

#endif // CENGINE_GUI_H_