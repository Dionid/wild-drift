#ifndef CENGINE_GUI_H_
#define CENGINE_GUI_H_

#include "raylib.h"
#include "node.h"
#include <functional>

enum class BtnState {
    Normal,
    Hover,
    Pressing
};

class Btn;

struct Callbacks {
    std::function<void(GameContext*, Btn*)> onHover;
    std::function<void(GameContext*, Btn*)> onDown;
    std::function<void(GameContext*, Btn*)> onUp;
    std::function<void(GameContext*, Btn*)> onClick;

    Callbacks(
        std::function<void(GameContext*, Btn*)> onHover = nullptr,
        std::function<void(GameContext*, Btn*)> onDown = nullptr,
        std::function<void(GameContext*, Btn*)> onUp = nullptr,
        std::function<void(GameContext*, Btn*)> onClick = nullptr
    ) {
        this->onHover = onHover;
        this->onDown = onDown;
        this->onUp = onUp;
        this->onClick = onClick;
    }
};

class Btn: public Node {
    public:
        BtnState state = BtnState::Normal;
        const char* text;
        int fontSize;
        Vector2 position;
        Vector2 anchor;
        Rectangle btnRect;
        Callbacks callbacks;

        Btn(
            const char* btnText,
            int btnTextFontSize,
            Vector2 position,
            cen::Size size,
            Vector2 anchor,
            Callbacks callbacks
        );

        void Render(GameContext* ctx);
};

#endif // CENGINE_GUI_H_