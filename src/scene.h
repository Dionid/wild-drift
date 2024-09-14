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
            Camera2D* camera,
            cen::Debugger debugger = cen::Debugger{},
            std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
            std::unique_ptr<cen::NodeStorage> nodeStorage = std::make_unique<cen::NodeStorage>(),
            std::unique_ptr<cen::RenderingEngine2D> renderingEngine = std::make_unique<cen::RenderingEngine2D>(),
            std::vector<std::unique_ptr<cen::TopicBase>> topics = std::vector<std::unique_ptr<cen::TopicBase>>(),
            std::unique_ptr<cen::EventBus> eventBus = std::make_unique<cen::EventBus>(),
            cen::PlayerInput playerInput = cen::PlayerInput{}
        ): cen::Scene(
            screen,
            camera,
            debugger,
            std::move(collisionEngine),
            std::move(nodeStorage),
            std::move(renderingEngine),
            std::move(topics),
            std::move(eventBus),
            playerInput
        ) {
            this->gameAudio = gameAudio;
        }

        void Init() {
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

            // ## Init Nodes
            for (const auto& node: this->nodeStorage->rootNodes) {
                node->TraverseInit();
            }

            // ## Node Storage
            this->nodeStorage->Init();
        }
};

#endif // MAIN_SCENE_H