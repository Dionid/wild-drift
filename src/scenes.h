#include "cengine/cengine.h"
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
            "MatchScene",
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->gameAudio = gameAudio;
        }

        void Init() override {
            // ## Match
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            PlaySound(this->gameAudio->start);

            DisableCursor();

            // ## MatchEndEvent
            auto onMatchEndEvent = [
                // this,
                // mainMenu,
                // matchEndMenu,
                // matchManager
            ](const cen::Event& event) {
                
            };

            this->eventBus.on(
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
            "MatchEndScene",
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {}

        void Init() override {
            // # Scene init
            // ## MatchEndMenu
            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>());

            this->eventBus.on(
                RestartEvent{},
                std::make_unique<cen::EventListener>(
                    [this](const cen::Event& event){
                        this->eventBus.emit(
                            cen::SceneChangeRequested{
                                "MatchScene"
                            }
                        );
                    }
                )
            );
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
            "MainMenuScene",
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
            ](const cen::Event& event) {
                std::printf("StartEvent\n");
                this->eventBus.emit(
                    cen::SceneChangeRequested{
                        "MatchScene"
                    }
                );
            };

            this->eventBus.on(
                StartEvent{},
                std::make_unique<cen::EventListener>(
                    onStartEvent
                )
            );

            // ## HostEvent
            auto onHostEvent = [
                this,
                mainMenu
            ](const cen::Event& event) {
                std::printf("HostEvent\n");
            };

            this->eventBus.on(
                HostEvent{},
                std::make_unique<cen::EventListener>(
                    onHostEvent
                )
            );

            // ## JoinEvent
            auto onJoinEvent = [
                this,
                mainMenu
            ](const cen::Event& event) {
                std::printf("JoinEvent\n");
            };

            this->eventBus.on(
                JoinEvent{},
                std::make_unique<cen::EventListener>(
                    onJoinEvent
                )
            );
        }
};

#endif // MAIN_SCENE_H