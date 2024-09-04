#ifndef CENGINE_GUI_H_
#define CENGINE_GUI_H_

#include "raylib.h"
#include "cengine/node.h"

enum class BtnState {
    Normal,
    Hover,
    Pressing
};

class Btn;

struct Callbacks {
    void (*onHover)(Btn* btn);
    void (*onDown)(Btn* btn);
    void (*onUp)(Btn* btn);
    void (*onClick)(Btn* btn);

    Callbacks(
        void (*onHover)(Btn* btn) = nullptr,
        void (*onDown)(Btn* btn) = nullptr,
        void (*onUp)(Btn* btn) = nullptr,
        void (*onClick)(Btn* btn) = nullptr
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
            Size size,
            Vector2 anchor,
            Callbacks callbacks
        );

        void Render(GameContext* ctx);
};

#endif // CENGINE_GUI_H_