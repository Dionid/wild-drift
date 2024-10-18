#include <ctime>
#include <thread>
#include <chrono>
#include "cengine/cengine.h"
#include "utils.h"
#include "audio.h"
#include "entity.h"
#include "match.h"
#include "menus.h"
#include "scenes.h"

void runSimulation(cen::SceneManager* sceneManager) {
    sceneManager->Run();
};

void runNetwork(cen::NetworkManager* networkManager) {
    networkManager->Run();
};

void runRendering(cen::RenderingEngine2D* renderingEngine) {
    renderingEngine->Run();
}

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "wild drift");
    SetTargetFPS(FPS);

    // # Audio
    InitAudioDevice();

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

    // # Network Manager
    cen::NetworkManager networkManager = cen::NetworkManager(
        120
    );

    // # Main Udp Transport
    cen::UdpTransport* mainTransport = networkManager.AddTransport(
        "main",
        std::make_unique<cen::UdpTransport>()
    );

    // # Scenes
    cen::SceneManager sceneManager = cen::SceneManager(
       &eventBus
    );

    // # Textures
    cen::TextureStorage textureStorage = {
        "map/"
    };

    textureStorage.AddTexture("top-down-test", "tileset_gray.png");

    // ## Storage
    CrossSceneStorage crossSceneStorage = {};

    // ## Main Menu Scene
    sceneManager.AddSceneConstructor(
        std::make_unique<cen::SceneConstructor>(
            MainMenuSceneName,
            [
                &crossSceneStorage,
                &camera,
                &renderingEngine,
                &eventBus
            ](){
                return std::make_unique<MainMenuScene>(
                    &crossSceneStorage,
                    cen::ScreenResolution{screenWidth, screenHeight},
                    &camera,
                    &renderingEngine,
                    &eventBus
                );
            }
        )
    );

    // ## Local Match Scene
    sceneManager.AddSceneConstructor(
        std::make_unique<cen::SceneConstructor>(
            LocalMatchSceneName,
            [
                &crossSceneStorage,
                &textureStorage,
                &camera,
                &renderingEngine,
                &eventBus
            ](){
                return std::make_unique<LocalMatchScene>(
                    &crossSceneStorage,
                    &textureStorage,
                    cen::ScreenResolution{screenWidth, screenHeight},
                    &camera,
                    &renderingEngine,
                    &eventBus
                );
            }
        )
    );

    // ## Set first scene
    sceneManager.SetFirstScene(MainMenuSceneName);

    // # Threads
    std::vector<std::thread> gameThreads;

    // ## Simulation thread
    gameThreads.push_back(std::thread(runSimulation, &sceneManager));

    // ## Network thread
    gameThreads.push_back(std::thread(runNetwork, &networkManager));

    // ## Rendering (main thread)
    runRendering(&renderingEngine);

    // # Exit
    // ## Stop signal
    networkManager.Stop();
    std::cout << "Stopping NetworkManager" << std::endl;
    sceneManager.Stop();
    std::cout << "Stopping SceneManager" << std::endl;

    // ## Join gameThreads after stop signal
    for (auto& thread: gameThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // ## Audio
    CloseAudioDevice();

    // ## Close window
    CloseWindow();

    // ## Success
    return 0;
}