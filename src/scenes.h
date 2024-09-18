#include "cengine/cengine.h"
#include "globals.h"
#include "menus.h"
#include "match.h"
#include "events.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

struct CrossSceneStorage {
    bool isPlayerWon;
};

class MatchScene: public cen::Scene {
    public:
        SpcAudio* gameAudio;
        CrossSceneStorage* crossSceneStorage;

        MatchScene(
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::Scene(
            MatchSceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            DisableCursor();
            
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            PlaySound(this->gameAudio->start);

            // ## MatchEndEvent
            auto onMatchEndEvent = [
                this
            ](const cen::Event* event) {
                auto matchEndEvent = static_cast<const MatchEndEvent*>(event);

                this->crossSceneStorage->isPlayerWon = matchEndEvent->isPlayerWon;

                this->eventBus.Emit(
                    std::make_unique<cen::SceneChangeRequested>(
                        MatchEndMenuSceneName
                    )
                );
            };

            this->eventBus.On(
                MatchEndEvent{},
                std::make_unique<cen::EventListener>(
                    onMatchEndEvent
                )
            );
        }
};

class MatchEndScene: public cen::Scene {
    public:
        SpcAudio* gameAudio;
        CrossSceneStorage* crossSceneStorage;

        MatchEndScene(
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::Scene(
            MatchEndMenuSceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            EnableCursor();

            std::cout << crossSceneStorage->isPlayerWon << std::endl;

            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>(
                crossSceneStorage->isPlayerWon
            ));

            // TODO: SoundManager (that will do nothing on server)
            if (crossSceneStorage->isPlayerWon) {
                PlaySound(this->gameAudio->win);
            } else {
                PlaySound(this->gameAudio->lost);
            }

            this->eventBus.On(
                RestartEvent{},
                std::make_unique<cen::EventListener>(
                    [this](const cen::Event* event){
                        this->eventBus.Emit(
                            std::make_unique<cen::SceneChangeRequested>(
                                MatchSceneName
                            )
                        );
                    }
                )
            );
        }
};

class MainMenuScene: public cen::Scene {
    public:
        cen::SceneManager* sceneManager;
        CrossSceneStorage* crossSceneStorage;
        SpcAudio* gameAudio;

        MainMenuScene(
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus,
            cen::SceneManager* sceneManager
        ): cen::Scene(
            MainMenuSceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->sceneManager = sceneManager;
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            EnableCursor();

            // ## MainMenu
            MainMenu* mainMenu = this->nodeStorage->AddNode(std::make_unique<MainMenu>());

            // # Events
            // ## StartEvent
            auto onStartEvent = [
                this
            ](const cen::Event* event) {
                this->eventBus.Emit(
                    std::make_unique<cen::SceneChangeRequested>(
                        MatchSceneName
                    )
                );
            };

            this->eventBus.On(
                StartEvent{},
                std::make_unique<cen::EventListener>(
                    onStartEvent
                )
            );

            // ## HostEvent
            auto onHostEvent = [
                this
            ](const cen::Event* event) {
                // this->sceneManager->ChangeScene(MatchSceneName);
                this->sceneManager->ChangeScene(
                    std::make_unique<MatchScene>(
                        this->crossSceneStorage,
                        this->gameAudio,
                        this->screen,
                        this->camera,
                        this->renderingEngine,
                        this->eventBus.parent
                    )
                );
            };

            this->eventBus.On(
                HostEvent{},
                std::make_unique<cen::EventListener>(
                    onHostEvent
                )
            );

            // ## JoinEvent
            auto onJoinEvent = [
                this,
                mainMenu
            ](const cen::Event* event) {
                std::printf("JoinEvent\n");
            };

            this->eventBus.On(
                JoinEvent{},
                std::make_unique<cen::EventListener>(
                    onJoinEvent
                )
            );
        }
};

#endif // MAIN_SCENE_H