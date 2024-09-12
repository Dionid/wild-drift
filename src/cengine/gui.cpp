
#include "gui.h"

namespace cen {

Btn::Btn(
    const char* btnText,
    int btnTextFontSize,
    Vector2 position,
    cen::Size size = cen::Size{ 0, 0 },
    Vector2 anchor = Vector2{ 1, 1 },
    Callbacks callbacks = Callbacks()
): cen::Node2D(position) {
    this->callbacks = callbacks;
    this->text = btnText;
    this->fontSize = btnTextFontSize;
    this->anchor = anchor;
    this->size = size;

    float width = size.width;
    float height = size.height;

    if (width == 0 || height == 0) {
        width = MeasureText(this->text, this->fontSize) * 2;
        height = (float)this->fontSize * 2;
    }

    this->btnRect = Rectangle{
        this->position.x - width * this->anchor.x,
        this->position.y - height * this->anchor.y,
        width,
        height
    };
};

void Btn::Update(cen::GameContext* ctx) {
    state = BtnState::Normal;

    Vector2 mousePoint = GetMousePosition();

    // Check button state
    if (CheckCollisionPointRec(mousePoint, btnRect))
    {
        state = BtnState::Hover;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            state = BtnState::Pressing;
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (this->callbacks.onClick != nullptr) {
                this->callbacks.onClick(ctx, this);
            }
        }
    }
}

}