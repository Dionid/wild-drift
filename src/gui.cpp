
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "gui.h"

int titleFontSize = 50;
const char* title = "Super Pong";

int startFontSize = 20;
const char* start = "Start";

enum class BtnState {
    Normal,
    Hover,
    Pressing
};

class Btn: public Node {
    public:
        BtnState state = BtnState::Normal;
        const char* text;
        int fontSize;
        Vector2 position;
        Vector2 anchor;
        Rectangle btnRect;

        Btn(
            const char* btnText,
            int btnTextFontSize,
            Vector2 position,
            Size size = Size{ 0, 0 },
            Vector2 anchor = Vector2{ 1, 1 }
        ): Node() {
            this->text = btnText;
            this->fontSize = btnTextFontSize;
            this->position = position;
            this->anchor = anchor;

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
        }

        void Render(GameContext* ctx) {
            Vector2 mousePoint = GetMousePosition();

            // Check button state
            if (CheckCollisionPointRec(mousePoint, btnRect))
            {
                state = BtnState::Hover;

                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    state = BtnState::Pressing;
                } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    // TODO: Callback
                }
            }

            switch (state) {
                case BtnState::Normal:
                    DrawRectangleRec(
                        btnRect,
                        ColorAlpha(WHITE, 0.5f)
                    );
                    break;
                case BtnState::Hover:
                    DrawRectangleRec(
                        btnRect,
                        ColorAlpha(WHITE, 0.75f)
                    );
                    break;
                case BtnState::Pressing:
                    DrawRectangleRec(
                        btnRect,
                        ColorAlpha(WHITE, 1.0f)
                    );
                    break;
            }

            DrawText(
                this->text,
                this->position.x - MeasureText(this->text, this->fontSize) * this->anchor.x,
                this->position.y - this->fontSize * this->anchor.y,
                this->fontSize,
                BLACK
            );
        }
};

void MainMenu::Render(GameContext* ctx) {
    DrawText(
        title,
        ctx->worldWidth / 2 - MeasureText(title, titleFontSize) / 2,
        ctx->worldHeight / 2 - titleFontSize / 2 - 30,
        titleFontSize,
        WHITE
    );

    // Draw button
    Btn btn = Btn(
        start,
        startFontSize,
        Vector2{
            ctx->worldWidth / 2,
            ctx->worldHeight / 2 + 50
        },
        Size{ 0, 0 },
        Vector2{ 0.5, 0.5 }
    );
    btn.Render(ctx);

    // float btnWidth = MeasureText(start, startFontSize) * 2;
    // auto btnRect = Rectangle{
    //     ctx->worldWidth / 2 - btnWidth / 2,
    //     ctx->worldHeight / 2 + titleFontSize / 2 + 15,
    //     btnWidth,
    //     (float)startFontSize * 2
    // };

    // BtnState btnState = BtnState::Normal;

    // Vector2 mousePoint = GetMousePosition();

    // // Check button state
    // if (CheckCollisionPointRec(mousePoint, btnRect))
    // {
    //     btnState = BtnState::Hover;

    //     if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    //         btnState = BtnState::Pressing;
    //     } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    //         // TODO: Start game
    //         // ...
    //     }
    // }

    // switch (btnState) {
    //     case BtnState::Normal:
    //         DrawRectangleRec(
    //             btnRect,
    //             ColorAlpha(WHITE, 0.5f)
    //         );
    //         break;
    //     case BtnState::Hover:
    //         DrawRectangleRec(
    //             btnRect,
    //             ColorAlpha(WHITE, 0.75f)
    //         );
    //         break;
    //     case BtnState::Pressing:
    //         DrawRectangleRec(
    //             btnRect,
    //             ColorAlpha(WHITE, 1.0f)
    //         );
    //         break;
    // }

    // DrawText(
    //     start,
    //     ctx->worldWidth / 2 - MeasureText(start, startFontSize) / 2,
    //     ctx->worldHeight / 2 + titleFontSize / 2 + 25,
    //     startFontSize,
    //     BLACK
    // );
};