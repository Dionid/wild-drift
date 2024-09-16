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
        StepLockNetworkManager* stepLockNetworkManager;

        MainScene(
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            StepLockNetworkManager* stepLockNetworkManager
        ): cen::Scene(
            screen,
            camera
        ) {
            this->gameAudio = gameAudio;
            this->stepLockNetworkManager = stepLockNetworkManager;
        }

        void Init() override {
            // # Scene init
            // ## MatchEndMenu
            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>());

            matchEndMenu->Deactivate();

            // ## Match
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio,
                [this]() {
                    this->eventBus->emit(OnMatchEndEvent());
                }
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
                this->stepLockNetworkManager->InitialSync();
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
        }
};

#endif // MAIN_SCENE_H