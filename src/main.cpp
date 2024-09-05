#include "cengine/cengine.h"
#include "utils.h"
#include "entity.h"
#include "match.h"
#include "menus.h"

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

    // # Match
    auto mm = scene.node_storage->AddNode(std::make_unique<MatchManager>(
        0,
        0
    ));

    // scene.node_storage->AddNode(std::make_unique<MainMenu>());
    // scene.node_storage->AddNode(std::make_unique<MatchEndMenu>());

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
    // # While nodes are initing more of them can be added
    for (int i = 0; i < scene.node_storage->nodes.size(); i++) {
        scene.node_storage->nodes[i]->TraverseInit(&ctx);
    }

    // # Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // ## Update
        //----------------------------------------------------------------------------------

        // ## Initial
        for (const auto& node: ctx.scene->node_storage->nodes) {
            node->TraverseUpdate(&ctx);
        }

        // ## Collision
        collisionEngine.NarrowCollisionCheckNaive(&ctx);

        //----------------------------------------------------------------------------------

        // ## Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            for (const auto& node: ctx.scene->node_storage->nodes) {
                node->TraverseRender(&ctx);
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