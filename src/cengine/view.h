#ifndef CENGINE_VIEW_H_
#define CENGINE_VIEW_H_

#include <raylib.h>
#include "node.h"
#include "node-2d.h"

// # Views

namespace cen {

class LineView: public cen::Node2D {
    public:
        float length;
        float alpha;
        Color color;

        LineView(Vector2 position, float length, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, zOrder, id, parent) {
            this->length = length;
            this->alpha = alpha;
            this->color = color;
        }

        void Render(cen::GameContext* ctx) override {
            auto globalPosition = this->GlobalPosition();

            Vector2 end = {
                globalPosition.x,
                globalPosition.y + this->length
            };
            DrawLineV(globalPosition, end, ColorAlpha(this->color, this->alpha));
        }
};

class CircleView: public cen::Node2D {
    public:
        float radius;
        float alpha;
        Color color;
        bool fill;

        CircleView(float radius, Vector2 position = Vector2{}, Color color = WHITE, float alpha = 1.0f, bool fill = true,  Vector2 globalPosition = Vector2{}, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, zOrder, id, parent) {
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }

        void Render(cen::GameContext* ctx) override {
            auto globalPosition = this->GlobalPosition();

            if (this->fill) {
                DrawCircleV(globalPosition, this->radius, ColorAlpha(this->color, this->alpha));
                return;
            }

            DrawCircleLinesV(globalPosition, this->radius, ColorAlpha(this->color, this->alpha));
        }
};

class RectangleView: public cen::Node2D {
    public:
        cen::Size size;
        Color color;
        float alpha;

        RectangleView(cen::Size size, Color color = WHITE, float alpha = 1.0f, Vector2 position = Vector2{}, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, zOrder, id, parent) {
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }

        void Render(cen::GameContext* ctx) override {
            auto globalPosition = this->GlobalPosition();

            DrawRectangle(globalPosition.x - this->size.width * 0.5, globalPosition.y - this->size.height * 0.5, this->size.width, this->size.height, ColorAlpha(this->color, this->alpha));
        }
};

} // namespace cen

#endif // CENGINE_VIEW_H_