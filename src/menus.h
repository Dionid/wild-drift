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
        std::function<void(cen::GameContext*)> onStart;

        MainMenu(
            std::function<void(cen::GameContext*)> onStart = std::function<void(cen::GameContext*)>()
        );

        void Render(cen::GameContext* ctx) override;
        void Init(cen::GameContext* ctx) override;
};

class MatchEndMenu: public cen::Node2D {
    public:
        const int titleFontSize = defaultTitleFontSize;
        const int btnFontSize = defaultBtnFontSize;
        bool playerWon = false;

        std::function<void(cen::GameContext*)> onRestart;

        MatchEndMenu(
            std::function<void(cen::GameContext*)> onRestart = std::function<void(cen::GameContext*)>(),
            bool playerWon = false
        );

        void Render(cen::GameContext* ctx) override;
        void Init(cen::GameContext* ctx) override;
};

#endif // CENGINE_MENUS_H_