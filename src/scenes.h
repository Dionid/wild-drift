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

class MatchScene: public cen::LocalScene {
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
        ): cen::LocalScene(
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

            // # Entities
            const float sixthScreen = screen.width/6.0f;
            
            // ## Ball
            float ballRadius = 15.0f;
            float randomAngle = (fixedSimulationTick % 100 / 100.0f) * 2 * PI;
            Ball* ball = this->nodeStorage->AddNode(
                std::make_unique<Ball>(
                    this->gameAudio,
                    ballRadius,
                    (Vector2){ screen.width/2.0f, screen.height/2.0f },
                    (cen::Size){ ballRadius*2, ballRadius*2 },
                    (Vector2){ cos(randomAngle) * 6, sin(randomAngle) * 6 },
                    10.0f
                )
            );

            ball->zOrder = 1;

            // ## Player
            Player* player = this->nodeStorage->AddNode(
                std::make_unique<Player>(
                    (Vector2){ sixthScreen, screen.height/2.0f },
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            player->zOrder = 1;

            // ## Enemy
            Enemy* enemy = this->nodeStorage->AddNode(
                std::make_unique<Enemy>(
                    ball->id,
                    (Vector2){ screen.width - sixthScreen, screen.height/2.0f },
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            enemy->zOrder = 1;

            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            matchManager->ballId = ball->id;
            matchManager->playerId = player->id;
            matchManager->enemyId = enemy->id;

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

class MatchLockStepScene: public cen::LockStepScene {
    public:
        SpcAudio* gameAudio;
        CrossSceneStorage* crossSceneStorage;

        MatchLockStepScene(
            bool isHost,
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LockStepScene(
            isHost,
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

            // # Entities
            const float sixthScreen = screen.width/6.0f;

            // ## Ball
            float ballRadius = 15.0f;
            float randomAngle = (fixedSimulationTick % 100 / 100.0f) * 2 * PI;
            Ball* ball = this->nodeStorage->AddNode(
                std::make_unique<Ball>(
                    this->gameAudio,
                    ballRadius,
                    (Vector2){ screen.width/2.0f, screen.height/2.0f },
                    (cen::Size){ ballRadius*2, ballRadius*2 },
                    (Vector2){ cos(randomAngle) * 6, sin(randomAngle) * 6 },
                    10.0f
                )
            );

            ball->zOrder = 1;

            // ## Player
            Player* player = this->nodeStorage->AddNode(
                std::make_unique<Player>(
                    (Vector2){ sixthScreen, screen.height/2.0f },
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            player->zOrder = 1;

            // ## Enemy
            Enemy* enemy = this->nodeStorage->AddNode(
                std::make_unique<Enemy>(
                    ball->id,
                    (Vector2){ screen.width - sixthScreen, screen.height/2.0f },
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            enemy->zOrder = 1;

            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            matchManager->ballId = ball->id;
            matchManager->playerId = player->id;
            matchManager->enemyId = enemy->id;

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

class MatchEndScene: public cen::LocalScene {
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
        ): cen::LocalScene(
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

class MainMenuScene: public cen::LocalScene {
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
        ): cen::LocalScene(
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
                    std::make_unique<MatchLockStepScene>(
                        true,
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