#include <ctime>
#include <thread>
#include <chrono>
#include "cengine/cengine.h"
#include "cengine/loader.h"
#include "utils.h"
#include "audio.h"
#include "entity.h"
#include "match.h"
#include "menus.h"
#include "game_tick.h"
#include "scenes.h"
#include "step_lock_manager.h"

void runSimulation(cen::Scene* scene) {
    scene->RunSimulation();
};

void runRendering(cen::RenderingEngine2D* renderingEngine) {
    renderingEngine->Run();
}

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    EnableCursor();
    SetTargetFPS(FPS);

    // # Audio
    InitAudioDevice();

    SpcAudio gameAudio = {
        Sound(
            LoadSound(cen::GetResourcePath("audio/start.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/hit.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/score.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/lost.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/win.wav").c_str())
        )
    };

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // # Rendering Engine
    cen::RenderingEngine2D renderingEngine = cen::RenderingEngine2D();

    // # Event Bus
    cen::EventBus eventBus = cen::EventBus(
        nullptr
    );

    // eventBus.on(
    //     StartEvent{},
    //     std::make_unique<cen::EventListener>(
    //         [](const cen::Event& event) {
    //             std::printf("TOP StartEvent\n");
    //         }
    //     )
    // );

    // # Scenes
    cen::SceneManager sceneManager = cen::SceneManager(
       &eventBus
    );

    // ## Main Menu Scene
    auto mainMenuScene = sceneManager.AddScene(
        std::make_unique<MainMenuScene>(
            cen::ScreenResolution{screenWidth, screenHeight},
            &camera,
            &renderingEngine,
            &eventBus
        )
    );

    sceneManager.ChangeCurrentScene(mainMenuScene->name);

    // ## Match Scene
    sceneManager.AddScene(
        std::make_unique<MatchEndScene>(
            cen::ScreenResolution{screenWidth, screenHeight},
            &camera,
            &renderingEngine,
            &eventBus
        )
    );

    // ## Match Scene
    sceneManager.AddScene(
        std::make_unique<MatchScene>(
            &gameAudio,
            cen::ScreenResolution{screenWidth, screenHeight},
            &camera,
            &renderingEngine,
            &eventBus
        )
    );

    // # Threads
    std::vector<std::thread> threads;

    // # Simulation Loop Thread
    threads.push_back(std::thread(runSimulation, mainMenuScene));

    // # Rendering Loop Thread
    runRendering(&renderingEngine);

    // # Exit
    // ## Stop signal
    sceneManager.StopTheWorld();

    // ## Join threads after stop signal
    for (auto& thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // ## Audio
    CloseAudioDevice();

    // ## Close window
    CloseWindow();

    return 0;
}