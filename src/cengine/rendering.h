#ifndef SPC_RENDERING_H
#define SPC_RENDERING_H

#include <memory>
#include <vector>
#include <map>
#include <cstring>
#include <mutex>
#include <atomic>
#include "core.h"
#include "node.h"
#include "view.h"
#include "gui.h"
#include "node_storage.h"
#include "debug.h"

namespace cen {

class CanvasItem2D {
    public:
        node_id_t id;
        int zOrder;
        Vector2 position;
        float alpha;

        CanvasItem2D(
            Vector2 position,
            float alpha,
            int zOrder,
            uint16_t id = 0
        ) {
            this->position = position;
            this->zOrder = zOrder;
            this->alpha = alpha;
            this->id = id;
        }

        virtual void Render() = 0;
};

class LineCanvasItem2D: public CanvasItem2D {
    public:
        float length;
        float alpha;
        Color color;

        LineCanvasItem2D(
            Vector2 position,
            float length,
            Color color = WHITE,
            float alpha = 1.0f,
            int zOrder = 0,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, id) {
            this->length = length;
            this->alpha = alpha;
            this->color = color;
        }

        void Render() override {            Vector2 end = {
                this->position.x,
                this->position.y + this->length
            };
            DrawLineV(this->position, end, ColorAlpha(this->color, this->alpha));
        }
};

class CircleCanvasItem2D: public CanvasItem2D {
    public:
        float radius;
        Color color;
        bool fill;

        CircleCanvasItem2D(
            Vector2 position,
            float radius,
            Color color = WHITE,
            float alpha = 1.0f,
            bool fill = true,
            int zOrder = 0,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, id) {
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }

        void Render() override {
            if (this->fill) {
                DrawCircleV(this->position, this->radius, ColorAlpha(this->color, this->alpha));
                return;
            }

            DrawCircleLinesV(this->position, this->radius, ColorAlpha(this->color, this->alpha));
        }
};

class RectangleCanvasItem2D: public CanvasItem2D {
    public:
        cen::Size size;
        Color color;

        RectangleCanvasItem2D(
            Vector2 position,
            cen::Size size,
            Color color = WHITE,
            float alpha = 1.0f,
            int zOrder = 0,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, id) {
            this->size = size;
            this->color = color;
        }

        void Render() override {
            DrawRectangle(this->position.x - this->size.width * 0.5, this->position.y - this->size.height * 0.5, this->size.width, this->size.height, ColorAlpha(this->color, this->alpha));
        }
};

class ButtonCanvasItem2D: public CanvasItem2D {
    public:
        cen::BtnState state = cen::BtnState::Normal;
        const char* text;
        int fontSize;
        Vector2 anchor;
        Rectangle btnRect;

        ButtonCanvasItem2D(
            Vector2 position,
            cen::BtnState state,
            const char* btnText,
            int btnTextFontSize,
            cen::Size size,
            Vector2 anchor,
            float alpha = 1.0f,
            int zOrder = 0,
            node_id_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, id) {
            this->state = state;
            this->text = btnText;
            this->fontSize = btnTextFontSize;
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

        void Render() override {            switch (state) {
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

class TextCanvasItem2D: public CanvasItem2D {
    public:
        std::string text;
        int fontSize;
        Color color;

        TextCanvasItem2D(
            Vector2 position,
            std::string text,
            int fontSize,
            Color color,
            float alpha = 1.0f,
            int zOrder = 0
        ): CanvasItem2D(position, alpha, zOrder) {
            this->text = text;
            this->fontSize = fontSize;
            this->color = color;
        }

        void Render() override {
            DrawText(
                this->text.c_str(),
                this->position.x,
                this->position.y,
                this->fontSize,
                this->color
            );
        }
};

#define render_buffer std::vector<std::unique_ptr<CanvasItem2D>>

class RenderingEngine2D {
    private:
        std::atomic<int> activeRenderBufferInd;
        std::mutex firstBufferMutex;
        std::mutex secondBufferMutex;

    public:
        render_buffer firstBuffer;
        render_buffer secondBuffer;

        void MapNode2D(
            render_buffer& activeRenderBuffer,
            cen::Node2D* node2D,
            float alpha
        ) {
            Vector2 position = node2D->GlobalPosition();
            Vector2 previousPosition = node2D->PreviousGlobalPosition();

            Vector2 newGlobalPosition = Vector2Lerp(
                previousPosition,
                position,
                alpha
            );

            if (auto lineView = dynamic_cast<cen::LineView*>(node2D)) {
                activeRenderBuffer.push_back(
                    std::make_unique<LineCanvasItem2D>(
                        newGlobalPosition,
                        lineView->length,
                        lineView->color,
                        lineView->alpha,
                        lineView->zOrder,
                        lineView->id
                    )
                );
            } else if (auto circleView = dynamic_cast<cen::CircleView*>(node2D)) {
                activeRenderBuffer.push_back(
                    std::make_unique<CircleCanvasItem2D>(
                        newGlobalPosition,
                        circleView->radius,
                        circleView->color,
                        circleView->alpha,
                        circleView->fill,
                        circleView->zOrder,
                        circleView->id
                    )
                );
            } else if (auto rectangleView = dynamic_cast<cen::RectangleView*>(node2D)) {
                activeRenderBuffer.push_back(
                    std::make_unique<RectangleCanvasItem2D>(
                        newGlobalPosition,
                        rectangleView->size,
                        rectangleView->color,
                        rectangleView->alpha,
                        rectangleView->zOrder,
                        rectangleView->id
                    )
                );
            } else if (auto buttonView = dynamic_cast<cen::Btn*>(node2D)) {
                activeRenderBuffer.push_back(
                    std::make_unique<ButtonCanvasItem2D>(
                        newGlobalPosition,
                        buttonView->state,
                        buttonView->text,
                        buttonView->fontSize,
                        buttonView->size,
                        buttonView->anchor
                    )
                );
            } else if (auto textView = dynamic_cast<cen::TextView*>(node2D)) {
                
                activeRenderBuffer.push_back(
                    std::make_unique<TextCanvasItem2D>(
                        newGlobalPosition,
                        std::string(textView->text).c_str(),
                        textView->fontSize,
                        textView->color
                    )
                );
            }
        }

        void SyncRenderBuffer(
            cen::NodeStorage* const nodeStorage,
            float alpha
        ) {
            auto writeBuffer = render_buffer();

            // # Sync with game Nodes
            for (auto const& node: nodeStorage->renderNodes) {
                if (node->AnyParentDeactivated()) {
                    continue;
                }

                this->MapNode2D(
                    writeBuffer,
                    node,
                    alpha
                );
            }

            std::sort(writeBuffer.begin(),writeBuffer.end(), [](const std::unique_ptr<cen::CanvasItem2D>& a, const std::unique_ptr<cen::CanvasItem2D>& b) {
                return a->zOrder < b->zOrder;
            });

            {
                if (activeRenderBufferInd.load(std::memory_order_acquire) == 0) {
                    // std::lock_guard<std::mutex> lock(secondBufferMutex);
                    secondBuffer = std::move(writeBuffer);
                    activeRenderBufferInd.store(1, std::memory_order_release);
                } else {
                    // std::lock_guard<std::mutex> lock(firstBufferMutex);
                    firstBuffer = std::move(writeBuffer);
                    activeRenderBufferInd.store(0, std::memory_order_release);
                }
            }
        }

        void Render() {
            if (activeRenderBufferInd.load(std::memory_order_acquire) == 0) {
                // std::lock_guard<std::mutex> lock(firstBufferMutex);
                for (const auto& item: firstBuffer) {
                    item->Render();
                }
            } else {
                // std::lock_guard<std::mutex> lock(secondBufferMutex);
                for (const auto& item: secondBuffer) {
                    item->Render();
                }
            }
        };

        int runPipeline(cen::Debugger* debugger) {
            while (!WindowShouldClose())    // Detect window close button or ESC key
            {
                BeginDrawing();
                    ClearBackground(BLACK);
                    this->Render();
                    debugger->Render();
                EndDrawing();
            }

            return EXIT_SUCCESS;
        }
};

}

#endif // SPC_RENDERING_H