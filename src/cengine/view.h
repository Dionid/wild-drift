#ifndef CENGINE_VIEW_H_
#define CENGINE_VIEW_H_

#include <raylib.h>
#include "node.h"
#include "node-2d.h"

// # Views

class LineView: public Node2D {
    public:
        float length;
        float alpha;
        Color color;

        LineView(Vector2 position, float length, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, uint16_t id = 0, Node* parent = nullptr): Node2D(position, id, parent) {
            this->length = length;
            this->alpha = alpha;
            this->color = color;
        }

        void Render(GameContext* ctx) override {
            auto globalPosition = this->GlobalPosition();

            Vector2 end = {
                globalPosition.x,
                globalPosition.y + this->length
            };
            DrawLineV(globalPosition, end, ColorAlpha(this->color, this->alpha));
        }
};

class CircleView: public Node2D {
    public:
        float radius;
        float alpha;
        Color color;
        bool fill;

        CircleView(float radius, Vector2 position = Vector2{}, Color color = WHITE, float alpha = 1.0f, bool fill = true,  Vector2 globalPosition = Vector2{}, uint16_t id = 0, Node* parent = nullptr): Node2D(position, id, parent) {
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }

        void Render(GameContext* ctx) override {
            auto globalPosition = this->GlobalPosition();

            if (this->fill) {
                DrawCircleV(globalPosition, this->radius, ColorAlpha(this->color, this->alpha));
                return;
            }

            DrawCircleLinesV(globalPosition, this->radius, ColorAlpha(this->color, this->alpha));
        }
};

class RectangleView: public Node2D {
    public:
        cen::Size size;
        Color color;
        float alpha;

        RectangleView(cen::Size size, Color color = WHITE, float alpha = 1.0f, Vector2 position = Vector2{}, uint16_t id = 0, Node* parent = nullptr): Node2D(position, id, parent) {
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }

        void Render(GameContext* ctx) override {
            auto globalPosition = this->GlobalPosition();

            DrawRectangle(globalPosition.x - this->size.width * 0.5, globalPosition.y - this->size.height * 0.5, this->size.width, this->size.height, ColorAlpha(this->color, this->alpha));
        }
};

#endif // CENGINE_VIEW_H_