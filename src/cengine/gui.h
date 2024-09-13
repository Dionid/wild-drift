#ifndef CENGINE_GUI_H_
#define CENGINE_GUI_H_

#include <functional>
#include "raylib.h"
#include "node_2d.h"

namespace cen {

enum class BtnState {
    Normal,
    Hover,
    Pressing
};

class Btn;

struct Callbacks {
    std::function<void(Btn*)> onHover;
    std::function<void(Btn*)> onDown;
    std::function<void(Btn*)> onUp;
    std::function<void(Btn*)> onClick;

    Callbacks(
        std::function<void(Btn*)> onHover = nullptr,
        std::function<void(Btn*)> onDown = nullptr,
        std::function<void(Btn*)> onUp = nullptr,
        std::function<void(Btn*)> onClick = nullptr
    ) {
        this->onHover = onHover;
        this->onDown = onDown;
        this->onUp = onUp;
        this->onClick = onClick;
    }
};

class Btn: public cen::Node2D {
    public:
        BtnState state = BtnState::Normal;
        const char* text;
        int fontSize;
        cen::Size size;
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

        void Update() override; 
};

}

#endif // CENGINE_GUI_H_