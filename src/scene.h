#include "cengine/cengine.h"
#include "menus.h"
#include "match.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

class MainScene: public cen::Scene {
    public:
        SpcAudio* gameAudio;

        MainScene(
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera
        ): cen::Scene(
            screen,
            camera
        ) {
            this->gameAudio = gameAudio;
        }

        void Init() override {
            // # Scene init
            // ## MatchEndMenu
            auto matchEndMenu = this->nodeStorage->AddNode(std::make_unique<MatchEndMenu>(
                [&]() {
                    this->eventBus->emit(RestartEvent());
                }
            ));

            matchEndMenu->Deactivate();

            // ## Match
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio,
                [this, matchManager, matchEndMenu]() {
                    matchManager->Deactivate();
                    matchEndMenu->SetPlayerWon(matchManager->playerScore > matchManager->enemyScore);
                    matchEndMenu->Activate();
                    EnableCursor();
                }
            ));

            matchManager->Deactivate();

            // ## MainMenu
            MainMenu* mainMenu = this->nodeStorage->AddNode(std::make_unique<MainMenu>(
                [this]() {
                    this->eventBus->emit(StartEvent());
                }
            ));

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
        }
};

#endif // MAIN_SCENE_H