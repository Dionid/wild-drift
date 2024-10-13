#ifndef CENGINE_MENUS_H_
#define CENGINE_MENUS_H_

#include <functional>
#include "cengine/node.h"

const int defaultTitleFontSize = 50;
const int defaultBtnFontSize = 20;

class MainMenu: public cen::Node2D {
    public:
        const int titleFontSize = defaultTitleFontSize;
        const int btnFontSize = defaultBtnFontSize;
        const char* title = "Super Pong";
        const char* start = "Start";

        MainMenu();

        void Init() override;
};

#endif // CENGINE_MENUS_H_