#include "cengine/cengine.h"
#include "globals.h"
#include "menus.h"
#include "match.h"
#include "events.h"
#include "multiplayer.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

struct CrossSceneStorage {
};

class MainMenuScene: public cen::LocalScene {
    public:
        cen::SceneManager* sceneManager;
        CrossSceneStorage* crossSceneStorage;

        MainMenuScene(
            cen::SceneManager* sceneManager,
            CrossSceneStorage* crossSceneStorage,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LocalScene(
            MainMenuSceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->sceneManager = sceneManager;
            this->crossSceneStorage = crossSceneStorage;
        }

        void Init() override;
};

#endif // MAIN_SCENE_H