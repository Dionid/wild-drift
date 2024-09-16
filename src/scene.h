#include "cengine/cengine.h"
#include "menus.h"
#include "match.h"
#include "step_lock_manager.h"
#include "events.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

class MainScene: public cen::Scene {
    public:
        SpcAudio* gameAudio;

        MainScene(
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine
        ): cen::Scene(
            screen,
            camera,
            renderingEngine
        ) {
            this->gameAudio = gameAudio;
        }

        void Init() override {
            // # Scene init
            // ## MatchEndMenu
            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>());

            matchEndMenu->Deactivate();

            // ## Match
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            matchManager->Deactivate();

            // ## MainMenu
            MainMenu* mainMenu = this->nodeStorage->AddNode(std::make_unique<MainMenu>());

            // # Events
            // ## StartEvent
            auto onStartEvent = [
                this,
                mainMenu,
                matchEndMenu,
                matchManager
            ](const cen::Event& event) {
                PlaySound(this->gameAudio->start);
                mainMenu->Deactivate();
                matchEndMenu->Deactivate();
                matchManager->Reset();
                matchManager->Activate();
                DisableCursor();
            };

            this->eventBus->on(
                StartEvent{},
                std::make_unique<cen::EventListener>(
                    onStartEvent
                )
            );

            this->eventBus->on(
                RestartEvent{},
                std::make_unique<cen::EventListener>(
                    onStartEvent
                )
            );

            // ## MatchEndEvent
            auto onMatchEndEvent = [
                this,
                mainMenu,
                matchEndMenu,
                matchManager
            ](const cen::Event& event) {
                matchManager->Deactivate();
                matchEndMenu->SetPlayerWon(matchManager->playerScore > matchManager->enemyScore);
                matchEndMenu->Activate();
                EnableCursor();
            };

            this->eventBus->on(
                OnMatchEndEvent{},
                std::make_unique<cen::EventListener>(
                    onMatchEndEvent
                )
            );

            // ## HostEvent
            auto onHostEvent = [
                this,
                mainMenu,
                matchEndMenu,
                matchManager
            ](const cen::Event& event) {
                // DisableCursor();
                // matchManager->InitMultiplayerMode(true);
                // PlaySound(this->gameAudio->start);
                // mainMenu->Deactivate();
                // matchEndMenu->Deactivate();
                // matchManager->Reset();
                // matchManager->Activate();
            };

            this->eventBus->on(
                HostEvent{},
                std::make_unique<cen::EventListener>(
                    onHostEvent
                )
            );
        }
};

#endif // MAIN_SCENE_H