#include "cengine/cengine.h"
#include "utils.h"
#include "entity.h"
#include "main_level.h"
#include "main_menu.h"

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    // DisableCursor();
    SetTargetFPS(FPS);

    Debugger debugger;

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // # Scene
    Scene scene;

    // # Level
    // auto mainLevel = MainLevel(
    //     screenWidth,
    //     screenHeight,
    //     &scene
    // );
    // mainLevel.InitMainLevel();

    scene.node_storage->AddNode(std::make_unique<MainMenu>());

    // # Collision Engine
    CollisionEngine collisionEngine;

    // # Game Context
    GameContext ctx = {
        &scene,
        &collisionEngine,
        screenWidth,
        screenHeight
    };

    // # Init
    for (const auto& node: ctx.scene->node_storage->nodes) {
        node->Init(&ctx);
    }

    // # Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // ## Update
        //----------------------------------------------------------------------------------

        // ## Initial
        for (const auto& node: ctx.scene->node_storage->nodes) {
            node->TraverseNodeUpdate(&ctx);
        }

        // ## Collision
        collisionEngine.NarrowCollisionCheckNaive(&ctx);

        //----------------------------------------------------------------------------------

        // ## Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (const auto& node: ctx.scene->node_storage->nodes) {
                node->TraverseNodeRender(&ctx);
            }
            debugger.Render(&ctx);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // ## De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}