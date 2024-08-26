#include <raylib.h>
#include "engine.h"
#include "nodes.h"

#ifndef CENGINE_VIEW_H
#define CENGINE_VIEW_H

// # Views

class LineView: public Node2D {
    public:
        float length;
        float alpha;
        Color color;

        LineView(Vector2 position, float length, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, std::shared_ptr<Node> parent = nullptr): Node2D(position, globalPosition, parent) {
            this->length = length;
            this->alpha = alpha;
            this->color = color;
        }

        void Render(GameContext* ctx, GameObject* thisGO) override {
            auto globalPosition = this->position;

            auto parent = dynamic_pointer_cast<Node2D>(this->parent);

            if (parent != nullptr) {
                globalPosition = Vector2Add(
                    globalPosition,
                    parent.get()->position
                );
            }

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

        CircleView(float radius, Vector2 position = Vector2{}, Color color = WHITE, float alpha = 1.0f, bool fill = true,  Vector2 globalPosition = Vector2{}, std::shared_ptr<Node> parent = nullptr): Node2D(position, globalPosition, parent) {
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }

        void Render(GameContext* ctx, GameObject* thisGO) override {
            if (this->fill) {
                DrawCircleV(this->globalPosition, this->radius, ColorAlpha(this->color, this->alpha));
                return;
            }

            DrawCircleLinesV(this->globalPosition, this->radius, ColorAlpha(this->color, this->alpha));
        }
};

class RectangleView: public Node2D {
    public:
        Size size;
        Color color;
        float alpha;

        RectangleView(Vector2 position, Size size, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, std::shared_ptr<Node> parent = nullptr): Node2D(position, globalPosition, parent) {
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }

        void Render(GameContext* ctx, GameObject* thisGO) override {
            DrawRectangle(this->position.x, this->position.y, this->size.width, this->size.height, ColorAlpha(this->color, this->alpha));
        }
};

#endif // CENGINE_VIEW_H