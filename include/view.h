#include <raylib.h>
#include "engine.h"

#ifndef CENGINE_VIEW_H
#define CENGINE_VIEW_H

// # Views

class LineView: public Node {
    public:
        Vector2 start;
        Vector2 end;
        float alpha;
        Color color;

        LineView(Vector2 start, Vector2 end, Color color = WHITE, float alpha = 1.0f) {
            this->start = start;
            this->end = end;
            this->alpha = alpha;
            this->color = color;
        }

        void Render(GameContext* ctx, GameObject* thisGO) override {
            DrawLineV(this->start, this->end, ColorAlpha(this->color, this->alpha));
        }
};

class CircleView: public Node {
    public:
        Vector2 center;
        float radius;
        float alpha;
        Color color;
        bool fill;

        CircleView(Vector2 center, float radius, Color color = WHITE, float alpha = 1.0f, bool fill = true) {
            this->center = center;
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }

        void Render(GameContext* ctx, GameObject* thisGO) override {
            if (this->fill) {
                DrawCircleV(this->center, this->radius, ColorAlpha(this->color, this->alpha));
                return;
            }

            DrawCircleLinesV(this->center, this->radius, ColorAlpha(this->color, this->alpha));
        }
};

class RectangleView: public Node {
    public:
        Vector2 position;
        Size size;
        Color color;
        float alpha;

        RectangleView(Vector2 position, Size size, Color color = WHITE, float alpha = 1.0f) {
            this->position = position;
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }

        void Render(GameContext* ctx, GameObject* thisGO) override {
            DrawRectangle(this->position.x, this->position.y, this->size.width, this->size.height, ColorAlpha(this->color, this->alpha));
        }
};

#endif // CENGINE_VIEW_H