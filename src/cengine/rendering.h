#ifndef CENGINE_RENDERING_H
#define CENGINE_RENDERING_H

#include <memory>
#include <vector>
#include <map>
#include <cstring>
#include <mutex>
#include <atomic>
#include "view.h"
#include "gui.h"
#include "node_storage.h"
#include "debug.h"

namespace cen {

#define render_buffer std::vector<std::unique_ptr<CanvasItem2D>>

class CanvasItem2D {
    public:
        node_id_t id;
        int zOrder;
        bool ySort;
        Vector2 position;
        float alpha;

        CanvasItem2D(
            Vector2 position,
            float alpha,
            int zOrder,
            bool ySort = false,
            uint16_t id = 0
        ) {
            this->position = position;
            this->zOrder = zOrder;
            this->alpha = alpha;
            this->id = id;
            this->ySort = ySort;
        }

        virtual ~CanvasItem2D() {}
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
            bool ySort = false,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, ySort, id) {
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
            bool ySort = false,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, ySort, id) {
            this->radius = radius;
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
            bool ySort = false,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, ySort, id) {
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
            bool ySort = false,
            node_id_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, ySort, id) {
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

        void Render() override {
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
            int zOrder = 0,
            bool ySort = false,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, ySort, id) {
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

class TextureCanvasItem2D: public CanvasItem2D {
    public:
        Texture* texture;
        int tileGID;
        int tilesetColumns;
        int tileWidth;
        int tileHeight;
        int x;
        int y;

        TextureCanvasItem2D(
            Texture* texture,
            Vector2 position,
            int tileGID,
            int tilesetColumns,
            int tileWidth,
            int tileHeight,
            float alpha = 1.0f,
            int zOrder = 0
        ): CanvasItem2D(position, alpha, zOrder) {
            this->texture = texture;
        }

        void Render() override {
            int tileIndex = tileGID - 1;  // Tiled GIDs start at 1
            int tilesetX = (tileIndex % tilesetColumns) * tileWidth;
            int tilesetY = (tileIndex / tilesetColumns) * tileHeight;

            Rectangle sourceRec = { (float)tilesetX, (float)tilesetY, (float)tileWidth, (float)tileHeight };
            Rectangle destRec = { (float)(position.x * tileWidth), (float)(position.y * tileHeight), (float)tileWidth, (float)tileHeight };
            DrawTexturePro(*texture, sourceRec, destRec, Vector2{0, 0}, 0.0f, WHITE);
        }
};

// # TileMapLayer

class TileCanvasItem2D: public CanvasItem2D {
    public:
        Rectangle texturePosition;
        Size size;
        Texture texture;

        TileCanvasItem2D(
            Texture texture,
            Rectangle texturePosition,
            Vector2 position,
            Size size,
            float alpha = 1.0f,
            int zOrder = 0,
            bool ySort = false,
            uint16_t id = 0
        ): CanvasItem2D(position, alpha, zOrder, ySort, id) {
            this->texturePosition = texturePosition;
            this->size = size;
            this->texture = texture;
        }

        void Render() override {
            DrawTexturePro(
                texture,
                texturePosition,
                Rectangle{
                    position.x,
                    position.y,
                    size.width,
                    size.height
                },
                Vector2{0, 0},
                0.0f,
                WHITE
            );
        }
};

// # Rendering Engine

class RenderingEngine2D {
    private:
        std::atomic<int> activeRenderBufferInd;

    public:
        render_buffer firstBuffer;
        render_buffer secondBuffer;
        Camera2D* camera;
        bool ySort = false;

        RenderingEngine2D(Camera2D* camera, bool ySort = false) {
            this->camera = camera;
            this->activeRenderBufferInd.store(0, std::memory_order_release);
            this->ySort = ySort;
        }

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
                        buttonView->anchor,
                        1.0f,
                        buttonView->zOrder,
                        buttonView->id
                    )
                );
            } else if (auto textView = dynamic_cast<cen::TextView*>(node2D)) {
                activeRenderBuffer.push_back(
                    std::make_unique<TextCanvasItem2D>(
                        newGlobalPosition,
                        std::string(textView->text).c_str(),
                        textView->fontSize,
                        textView->color,
                        1.0f,
                        textView->zOrder,
                        textView->ySort,
                        textView->id
                    )
                );
            } else if (auto tileView = dynamic_cast<cen::TileView*>(node2D)) {
                activeRenderBuffer.push_back(
                    std::make_unique<TileCanvasItem2D>(
                        tileView->texture,
                        tileView->texturePosition,
                        newGlobalPosition,
                        tileView->size,
                        1.0f,
                        tileView->zOrder,
                        tileView->ySort,
                        tileView->id
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

            std::sort(writeBuffer.begin(), writeBuffer.end(), [](const std::unique_ptr<cen::CanvasItem2D>& a, const std::unique_ptr<cen::CanvasItem2D>& b) {
                return a->zOrder < b->zOrder;
            });

            {
                if (activeRenderBufferInd.load(std::memory_order_acquire) == 0) {
                    secondBuffer = std::move(writeBuffer);
                    activeRenderBufferInd.store(1, std::memory_order_release);
                } else {
                    firstBuffer = std::move(writeBuffer);
                    activeRenderBufferInd.store(0, std::memory_order_release);
                }
            }
        }

        void Render() {
            if (activeRenderBufferInd.load(std::memory_order_acquire) == 0) {
                for (const auto& item: firstBuffer) {
                    item->Render();
                }
            } else {
                for (const auto& item: secondBuffer) {
                    item->Render();
                }
            }
        };

        int Run() {
            // TODO: Different way to pass debugger
            cen::Debugger debugger;
            while (!WindowShouldClose())    // Detect window close button or ESC key
            {
                BeginDrawing();
                    ClearBackground(BLACK);
                    BeginMode2D(*camera);
                    this->Render();
                    EndMode2D();
                    debugger.Render();
                EndDrawing();
            }

            return EXIT_SUCCESS;
        }
};

}

#endif // CENGINE_RENDERING_H