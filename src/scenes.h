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
            cen::UdpTransport* udpTransport,
            CrossSceneStorage* crossSceneStorage,
            SpcAudio* gameAudio,
            cen::ScreenResolution screen,
            Camera2D* camera,
            cen::RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus
        ): cen::LockStepScene(
            udpTransport,
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
            Opponent* enemy = this->nodeStorage->AddNode(
                std::make_unique<Opponent>(
                    cen::player_id_t(2),
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
        cen::UdpTransport* udpTransport;

        MainMenuScene(
            cen::UdpTransport* udpTransport,
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
            this->udpTransport = udpTransport;
            this->sceneManager = sceneManager;
            this->crossSceneStorage = crossSceneStorage;
            this->gameAudio = gameAudio;
        }

        ~MainMenuScene() {
            std::cout << "MainMenuScene destructor" << std::endl;
        }

        void BeforeStop() override {
            std::cout << "BeforeStop" << std::endl;
        }

        void Init() override;
};

// # ServerLobbyScene
class ServerLobbyScene: public cen::LocalScene {
    public:
        SpcAudio* gameAudio;
        cen::UdpTransport* udpTransport;
        int listenerId;
        std::unique_ptr<cen::MultiplayerNetworkTransport> multiplayerNetworkTransport;

        ServerLobbyScene(
            cen::UdpTransport* udpTransport,
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
            this->udpTransport = udpTransport;
            this->gameAudio = gameAudio;
            this->multiplayerNetworkTransport = std::make_unique<cen::MultiplayerNetworkTransport>(udpTransport);
        }

        void BeforeStop() override {}

        void Init() override {
            multiplayerNetworkTransport->OnMessageReceived(
                cen::OnMultiplayerMessageReceivedListener(
                    [this](cen::ReceivedMultiplayerNetworkMessage message){
                        std::cout << "Received client message " << static_cast<int>(message.message.type) << std::endl;

                        // this->multiplayerNetworkTransport->SendMessage(
                        //     cen::MultiplayerNetworkMessage{
                        //         .type = cen::MultiplayerNetworkMessageType::PLAYER_JOIN_SUCCESS,
                        //         .content = {}
                        //     }
                        // );
                    }
                )
            );

            this->udpTransport->InitAsServer(
                1234
            );

            this->nodeStorage->AddNode(std::make_unique<ServerLobbyMenu>());
        }
};

// # ClientLobbyScene
class ClientLobbyScene: public cen::LocalScene {
    public:
        SpcAudio* gameAudio;
        cen::UdpTransport* udpTransport;
        int listenerId;
        std::unique_ptr<cen::MultiplayerNetworkTransport> multiplayerNetworkTransport;

        ClientLobbyScene(
            cen::UdpTransport* udpTransport,
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
            this->udpTransport = udpTransport;
            this->gameAudio = gameAudio;
            this->multiplayerNetworkTransport = std::make_unique<cen::MultiplayerNetworkTransport>(udpTransport);
        }

        ~ClientLobbyScene() {
            std::cout << "ClientLobbyScene destructor" << std::endl;
        }

        void BeforeStop() override {}

        void Init() override {
            // this->listenerId = this->udpTransport->OnMessageReceived(
            //     std::make_unique<cen::OnMessageReceivedListener>(
            //         [this](cen::ReceivedNetworkMessage message){
            //             std::cout << "Received server message: " << static_cast<int>(message.type) << std::endl;
            //         }
            //     )
            // );

            multiplayerNetworkTransport->OnMessageReceived(
                cen::OnMultiplayerMessageReceivedListener(
                    [this](cen::ReceivedMultiplayerNetworkMessage message){
                        std::cout << "Received server message: " << static_cast<int>(message.message.type) << std::endl;

                        if (message.message.type != cen::MultiplayerNetworkMessageType::CONNECTED_TO_SERVER) {
                            return;
                        }
                        
                        // this->multiplayerNetworkTransport->SendMessage(
                        //     cen::MultiplayerNetworkMessage{
                        //         .type = cen::MultiplayerNetworkMessageType::PLAYER_JOIN_REQUEST,
                        //         .content = {}
                        //     }
                        // );

                        // std::cout << "Sent player join request" << std::endl;
                    }
                )
            );

            this->udpTransport->InitAsClient(
                "127.0.0.1",
                1234
            );

            this->nodeStorage->AddNode(std::make_unique<ServerLobbyMenu>());
        }
};

#endif // MAIN_SCENE_H