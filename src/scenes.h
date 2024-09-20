#include "cengine/cengine.h"
#include "globals.h"
#include "menus.h"
#include "match.h"
#include "events.h"
#include "multiplayer.h"

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

struct CrossSceneStorage {
    bool isPlayerWon;
};

class LocalMatchScene: public cen::LocalScene {
    public:
        SpcAudio* gameAudio;
        CrossSceneStorage* crossSceneStorage;

        LocalMatchScene(
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LocalScene(
            LocalMatchSceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            // DisableCursor();

            // # Entities
            const float sixthScreen = screen.width/6.0f;
            
            // ## Ball
            float ballRadius = 15.0f;
            // float randomAngle = (fixedSimulationTick % 100 / 100.0f) * 2 * PI;
            Ball* ball = this->nodeStorage->AddNode(
                std::make_unique<Ball>(
                    this->gameAudio,
                    ballRadius,
                    (Vector2){ screen.width/2.0f, screen.height/2.0f },
                    (cen::Size){ ballRadius*2, ballRadius*2 },
                    // (Vector2){ cos(randomAngle) * 6, sin(randomAngle) * 6 },
                    (Vector2){ 0, 0 },
                    10.0f
                )
            );

            ball->zOrder = 1;

            // ## Player
            Player* player = this->nodeStorage->AddNode(
                std::make_unique<Player>(
                    true,
                    0,
                    (Vector2){ sixthScreen, screen.height/2.0f },
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            player->zOrder = 1;

            // ## Enemy
            AiOpponent* enemy = this->nodeStorage->AddNode(
                std::make_unique<AiOpponent>(
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
            SpcMultiplayer* multiplayer,
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LockStepScene(
            LockStepMatchSceneName,
            std::make_unique<cen::LockStepNetworkManager>(
                multiplayer->multiplayerNetworkTransport.get(),
                multiplayer->currentPlayerId,
                std::vector<cen::player_id_t>{ multiplayer->opponentPlayerId }
            ),
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            // DisableCursor();

            bool isHost = this->lockStepNetworkManager->transport->udpTransport->isServer;
            Vector2 leftSidePosition = (Vector2){ screen.width/6.0f, screen.height/2.0f };
            Vector2 rightSidePosition = (Vector2){ screen.width - screen.width/6.0f, screen.height/2.0f };

            // # Entities
            const float sixthScreen = screen.width/6.0f;

            // ## Ball
            float ballRadius = 15.0f;

            Ball* ball = this->nodeStorage->AddNode(
                std::make_unique<Ball>(
                    this->gameAudio,
                    ballRadius,
                    (Vector2){ screen.width/2.0f, screen.height/2.0f },
                    (cen::Size){ ballRadius*2, ballRadius*2 },
                    (Vector2){ 0, 0 },
                    10.0f
                )
            );

            ball->zOrder = 1;

            // ## Player
            Player* player = this->nodeStorage->AddNode(
                std::make_unique<Player>(
                    isHost,
                    this->lockStepNetworkManager->currentPlayerId,
                    isHost ? leftSidePosition : rightSidePosition,
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            player->zOrder = 1;

            // ## Opponent
            Player* opponent = this->nodeStorage->AddNode(
                std::make_unique<Player>(
                    !isHost,
                    // TODO: Change this
                    this->lockStepNetworkManager->connectedPlayers[0],
                    !isHost ? leftSidePosition : rightSidePosition,
                    (cen::Size){ 40.0f, 120.0f },
                    (Vector2){ 0.0f, 0.0f },
                    1.5f,
                    10.0f
                )
            );

            opponent->zOrder = 1;

            // ## MatchManager
            MatchManager* matchManager = this->nodeStorage->AddNode(std::make_unique<MatchManager>(
                this->gameAudio
            ));

            matchManager->ballId = ball->id;
            matchManager->playerId = player->id;
            matchManager->enemyId = opponent->id;

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
                                LocalMatchSceneName
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
        SpcMultiplayer* scpMultiplayer;

        MainMenuScene(
            SpcMultiplayer* scpMultiplayer,
            cen::SceneManager* sceneManager,
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
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
            this->scpMultiplayer = scpMultiplayer;
            this->sceneManager = sceneManager;
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        void Init() override;
};

// # ServerLobbyScene
class ServerLobbyScene: public cen::LocalScene {
    public:
        SpcAudio* gameAudio;
        SpcMultiplayer* scpMultiplayer;

        ServerLobbyScene(
            SpcMultiplayer* scpMultiplayer,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LocalScene(
            ServerLobbySceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->scpMultiplayer = scpMultiplayer;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            this->scpMultiplayer->InitAsServer();

            this->nodeStorage->AddNode(std::make_unique<ServerLobbyMenu>());
        }
};

// # ClientLobbyScene
class ClientLobbyScene: public cen::LocalScene {
    public:
        SpcAudio* gameAudio;
        SpcMultiplayer* scpMultiplayer;

        ClientLobbyScene(
            SpcMultiplayer* scpMultiplayer,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LocalScene(
            ServerLobbySceneName,
            screen,
            camera,
            renderingEngine,
            eventBus
        ) {
            this->scpMultiplayer = scpMultiplayer;
            this->gameAudio = gameAudio;
        }

        void Init() override {
            this->scpMultiplayer->InitAsClient();

            this->nodeStorage->AddNode(std::make_unique<ServerLobbyMenu>());
        }
};

#endif // MAIN_SCENE_H