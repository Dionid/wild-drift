#include "cengine/cengine.h"
#include "memory"

#ifndef SPC_MULTIPLAYER_H
#define SPC_MULTIPLAYER_H

class SpcMultiplayer {
    public:
        cen::UdpTransport* udpTransport;
        std::unique_ptr<cen::MultiplayerNetworkTransport> multiplayerNetworkTransport;

        int currentPlayerId = -1;
        int opponentPlayerId = -1;

        SpcMultiplayer(
            cen::UdpTransport* udpTransport
        ) {
            this->udpTransport = udpTransport;
            this->multiplayerNetworkTransport = std::make_unique<cen::MultiplayerNetworkTransport>(udpTransport);
        }

        void Deinit() {
            this->currentPlayerId = -1;
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
                        std::cout << "Received server message: " << static_cast<int>(message.message.type) << std::endl;

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

                                std::cout << "Received player join success message: " << playerJoinSuccessMessage.newPlayerId << std::endl;

                                this->currentPlayerId = playerJoinSuccessMessage.newPlayerId;
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
            this->currentPlayerId = 1;

            multiplayerNetworkTransport->OnMessageReceived(
                cen::OnMultiplayerMessageReceivedListener(
                    [this](cen::ReceivedMultiplayerNetworkMessage message){
                        std::cout << "Received client message " << static_cast<int>(message.message.type) << std::endl;

                        switch ( message.message.type ) {
                            case cen::MultiplayerNetworkMessageType::PLAYER_JOIN_REQUEST: {
                                this->multiplayerNetworkTransport->SendMessage(
                                    cen::PlayerJoinSuccessMessage(
                                        2
                                    ).ToMultiplayerNetworkMessage()
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