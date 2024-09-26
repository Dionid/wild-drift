#include "cengine/cengine.h"
#include "memory"

#ifndef SPC_MULTIPLAYER_H
#define SPC_MULTIPLAYER_H

class SpcMultiplayer {
    public:
        cen::UdpTransport* udpTransport;
        std::unique_ptr<cen::MultiplayerNetworkTransport> multiplayerNetworkTransport;
        cen::EventBus eventBus;

        cen::player_id_t localPlayerId = -1;
        cen::player_id_t opponentPlayerId = -1;

        SpcMultiplayer(
            cen::UdpTransport* udpTransport,
            cen::EventBus* eventBus 
        ): eventBus(eventBus) {
            this->udpTransport = udpTransport;
            this->multiplayerNetworkTransport = std::make_unique<cen::MultiplayerNetworkTransport>(udpTransport);
        }

        void Deinit() {
            this->localPlayerId = -1;
            this->opponentPlayerId = -1;
            this->udpTransport->Deinit();
        }

        void InitAsClient(
            std::string customServerHost = "127.0.0.1",
            uint64_t customServerPort = 0
        ) {
            this->opponentPlayerId = 1;

            multiplayerNetworkTransport->OnMessageReceived(
                cen::OnMultiplayerMessageReceivedListener(
                    [this](cen::ReceivedMultiplayerNetworkMessage message){
                        switch (message.message.type) {
                            case cen::MultiplayerNetworkMessageType::CONNECTED_TO_SERVER: {
                                this->multiplayerNetworkTransport->SendMessage(
                                    cen::MultiplayerNetworkMessage{
                                        .type = cen::MultiplayerNetworkMessageType::PLAYER_JOIN_REQUEST,
                                        .content = {}
                                    }
                                );
                                break;
                            }
                            case cen::MultiplayerNetworkMessageType::PLAYER_JOIN_SUCCESS: {
                                cen::PlayerJoinSuccessMessage playerJoinSuccessMessage = cen::PlayerJoinSuccessMessage::FromMultiplayerNetworkMessage(message.message);

                                this->localPlayerId = playerJoinSuccessMessage.clientPlayerId;
                                this->opponentPlayerId = playerJoinSuccessMessage.serverPlayerId;

                                this->multiplayerNetworkTransport->SendMessage(
                                    cen::MultiplayerNetworkMessage{
                                        .type = cen::MultiplayerNetworkMessageType::READY_TO_START,
                                        .content = {}
                                    }
                                );
                                break;
                            }
                            case cen::MultiplayerNetworkMessageType::START_GAME: {
                                this->eventBus.Emit(
                                    std::make_unique<cen::SceneChangeRequested>(
                                        LockStepMatchSceneName
                                    )
                                );
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                )
            );

            this->udpTransport->InitAsClient(
                customServerHost,
                customServerPort
            );
        }

        void InitAsServer(
            int customServerPort = 1234
        ) {
            this->localPlayerId = 1;
            this->opponentPlayerId = 2;

            multiplayerNetworkTransport->OnMessageReceived(
                cen::OnMultiplayerMessageReceivedListener(
                    [this](cen::ReceivedMultiplayerNetworkMessage message){
                        switch ( message.message.type ) {
                            case cen::MultiplayerNetworkMessageType::PLAYER_JOIN_REQUEST: {
                                this->multiplayerNetworkTransport->SendMessage(
                                    cen::PlayerJoinSuccessMessage(
                                        this->localPlayerId,
                                        this->opponentPlayerId
                                    ).ToMultiplayerNetworkMessage()
                                );
                                break;
                            }
                            case cen::MultiplayerNetworkMessageType::READY_TO_START: {
                                this->multiplayerNetworkTransport->SendMessage(
                                    cen::MultiplayerNetworkMessage{
                                        .type = cen::MultiplayerNetworkMessageType::START_GAME,
                                        .content = {}
                                    }
                                );

                                this->eventBus.Emit(
                                    std::make_unique<cen::SceneChangeRequested>(
                                        LockStepMatchSceneName
                                    )
                                );
                                break;
                            }
                            case cen::MultiplayerNetworkMessageType::PLAYER_LEFT: {
                                this->eventBus.Emit(
                                    std::make_unique<cen::SceneChangeRequested>(
                                        MainMenuSceneName
                                    )
                                );
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                )
            );

            this->udpTransport->InitAsServer(
                customServerPort
            );
        }

};

#endif // SPC_MULTIPLAYER_H