#ifndef CENGINE_MENUS_H_
#define CENGINE_MENUS_H_

#include <functional>
#include "cengine/node.h"

const int defaultTitleFontSize = 50;
const int defaultBtnFontSize = 20;

class MainMenu: public Node {
    public:
        const int titleFontSize = defaultTitleFontSize;
        const int btnFontSize = defaultBtnFontSize;
        const char* title = "Super Pong";
        const char* start = "Start";
        std::function<void(GameContext*)> onStart;

        MainMenu(
            std::function<void(GameContext*)> onStart = std::function<void(GameContext*)>()
        );

        void Render(GameContext* ctx) override;
        void Init(GameContext* ctx) override;
};

class MatchEndMenu: public Node {
    public:
        const int titleFontSize = defaultTitleFontSize;
        const int btnFontSize = defaultBtnFontSize;
        bool playerWon = false;

        std::function<void(GameContext*)> onRestart;

        MatchEndMenu(
            std::function<void(GameContext*)> onRestart = std::function<void(GameContext*)>(),
            bool playerWon = false
        );

        void Render(GameContext* ctx) override;
        void Init(GameContext* ctx) override;
};

#endif // CENGINE_MENUS_H_