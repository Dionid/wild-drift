#include "cengine/cengine.h"
#include "globals.h"
#include "menus.h"
#include "match.h"
#include "step_lock_manager.h"
#include "events.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

class MatchScene: public cen::Scene {
    public:
        SpcAudio* gameAudio;

        MatchScene(
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
                this->eventBus.Emit(
                    std::make_unique<cen::SceneChangeRequested>(
                        MatchEndMenuSceneName
                    )
                );
            };

            this->eventBus.On(
                OnMatchEndEvent{},
                std::make_unique<cen::EventListener>(
                    onMatchEndEvent
                )
            );
        }
};

class MatchEndScene: public cen::Scene {
    public:
        MatchEndScene(
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
        ) {}

        void Init() override {
            EnableCursor();

            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>());

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

            // if (this->playerScore > this->enemyScore) {
            //     // TODO: SoundManager (that will do nothing on server)
            //     PlaySound(this->gameAudio->win);
            // } else {
            //     // TODO: SoundManager (that will do nothing on server)
            //     PlaySound(this->gameAudio->lost);
            // }
        }
};

class MainMenuScene: public cen::Scene {
    public:
        MainMenuScene(
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::Scene(
            MainMenuSceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {}

        void Init() override {
            // ## MainMenu
            MainMenu* mainMenu = this->nodeStorage->AddNode(std::make_unique<MainMenu>());

            // # Events
            // ## StartEvent
            auto onStartEvent = [
                this
            ](const cen::Event* event) {
                std::printf("StartEvent\n");
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
                this,
                mainMenu
            ](const cen::Event* event) {
                std::printf("HostEvent\n");
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