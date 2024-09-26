
#include <string>
#include "network.h"

#ifndef CEN_MULTIPLAYER_H
#define CEN_MULTIPLAYER_H

namespace cen {

enum class MultiplayerNetworkMessageType {
    // # Commands
    PLAYER_JOIN_REQUEST,
    PLAYER_JOIN_SUCCESS,
    START_GAME,
    
    // # Data
    GAME_DATA,

    // # Events
    CONNECTED_TO_SERVER,
    DISCONNECTED_FROM_SERVER,

    PLAYER_CONNECTED,
    PLAYER_LEFT,

    READY_TO_START,
};

struct MultiplayerNetworkMessage {
    MultiplayerNetworkMessageType type;
    std::vector<uint8_t> content;

    std::vector<uint8_t> Serialize() {
        std::vector<uint8_t> buffer;

        // Serialize MessageType as raw bytes
        MultiplayerNetworkMessageType typeCopy = type;
        uint8_t* typePtr = reinterpret_cast<uint8_t*>(&typeCopy);
        buffer.insert(buffer.end(), typePtr, typePtr + sizeof(MultiplayerNetworkMessageType));

        // Serialize the length of the content vector
        uint32_t contentLength = static_cast<uint32_t>(content.size());
        uint8_t* lengthPtr = reinterpret_cast<uint8_t*>(&contentLength);
        buffer.insert(buffer.end(), lengthPtr, lengthPtr + sizeof(uint32_t));

        // Serialize the content itself (as bytes)
        buffer.insert(buffer.end(), content.begin(), content.end());

        return buffer;
    }

    static std::optional<MultiplayerNetworkMessage> Deserialize(const std::vector<uint8_t>& data) {
        MultiplayerNetworkMessage message;

        size_t offset = 0;

        // Deserialize MessageType
        if (data.size() < sizeof(MultiplayerNetworkMessageType)) {
            // throw std::runtime_error("Insufficient data to deserialize MultiplayerNetworkMessageType");
            return std::nullopt;
        }
        std::memcpy(&message.type, data.data() + offset, sizeof(MultiplayerNetworkMessageType));
        offset += sizeof(MultiplayerNetworkMessageType);

        // Deserialize the length of the content vector
        if (data.size() < offset + sizeof(uint32_t)) {
            // throw std::runtime_error("Insufficient data to deserialize content length");
            return std::nullopt;
        }
        uint32_t contentLength = 0;
        std::memcpy(&contentLength, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Deserialize the content itself
        if (data.size() < offset + contentLength) {
            // throw std::runtime_error("Insufficient data to deserialize content");
            return std::nullopt;
        }
        message.content = std::vector<uint8_t>(data.begin() + offset, data.begin() + offset + contentLength);

        return message;
    }
};

struct PlayerJoinSuccessMessage {
    cen::player_id_t serverPlayerId;
    cen::player_id_t clientPlayerId;

    PlayerJoinSuccessMessage() {};

    PlayerJoinSuccessMessage(
        cen::player_id_t serverPlayerId,
        cen::player_id_t clientPlayerId
    ) {
        this->serverPlayerId = serverPlayerId;
        this->clientPlayerId = clientPlayerId;
    }

    MultiplayerNetworkMessage ToMultiplayerNetworkMessage() {
        std::vector<uint8_t> buffer;

        // Serialize serverPlayerId as raw bytes
        cen::player_id_t serverPlayerIdCopy = serverPlayerId;
        uint8_t* serverPlayerIdPtr = reinterpret_cast<uint8_t*>(&serverPlayerIdCopy);
        buffer.insert(buffer.end(), serverPlayerIdPtr, serverPlayerIdPtr + sizeof(cen::player_id_t));

        // Serialize clientPlayerId as raw bytes
        cen::player_id_t clientPlayerIdCopy = clientPlayerId;
        uint8_t* clientPlayerIdPtr = reinterpret_cast<uint8_t*>(&clientPlayerIdCopy);
        buffer.insert(buffer.end(), clientPlayerIdPtr, clientPlayerIdPtr + sizeof(cen::player_id_t));

        return MultiplayerNetworkMessage{
            .type = MultiplayerNetworkMessageType::PLAYER_JOIN_SUCCESS,
            .content = buffer
        };
    }

    static PlayerJoinSuccessMessage FromMultiplayerNetworkMessage(const MultiplayerNetworkMessage& message) {
        PlayerJoinSuccessMessage playerJoinSuccessMessage;

        size_t offset = 0;

        // Deserialize serverPlayerId
        if (message.content.size() < sizeof(cen::player_id_t)) {
            throw std::runtime_error("Insufficient data to deserialize serverPlayerId");
        }
        std::memcpy(&playerJoinSuccessMessage.serverPlayerId, message.content.data() + offset, sizeof(cen::player_id_t));
        offset += sizeof(cen::player_id_t);

        // Deserialize clientPlayerId
        if (message.content.size() < sizeof(cen::player_id_t)) {
            throw std::runtime_error("Insufficient data to deserialize clientPlayerId");
        }
        std::memcpy(&playerJoinSuccessMessage.clientPlayerId, message.content.data() + offset, sizeof(cen::player_id_t));
        offset += sizeof(cen::player_id_t);

        return playerJoinSuccessMessage;
    }
};

struct ReceivedMultiplayerNetworkMessage {
    MultiplayerNetworkMessage message;
    ReceivedNetworkMessage origin;
};

struct OnMultiplayerMessageReceivedListener: public Listener<ReceivedMultiplayerNetworkMessage> {
    OnMultiplayerMessageReceivedListener(
        std::function<void(ReceivedMultiplayerNetworkMessage)> trigger,
        int id = 0
    ): Listener(
        trigger,
        id
    ) {}
};

class MultiplayerNetworkTransport {
    public:
        std::atomic<bool> isRunning = false;
        cen::UdpTransport* udpTransport;
        std::vector<int> listenersIds;

        MultiplayerNetworkTransport(
            cen::UdpTransport* udpTransport
        ) {
            this->udpTransport = udpTransport;
        }

        ~MultiplayerNetworkTransport() {
            for (const auto& listenerId: this->listenersIds) {
                this->udpTransport->OffMessageReceived(listenerId);
            }
        }

        int OnMessageReceived(
            OnMultiplayerMessageReceivedListener onMessageReceived
        ) {
            int newId = this->udpTransport->OnMessageReceived(
                std::make_unique<OnMessageReceivedListener>(
                    [this, onMessageReceived](ReceivedNetworkMessage message) {
                        switch (message.type) {
                            case ReceivedNetworkMessageType::CONNECTED_TO_SERVER: {
                                onMessageReceived.trigger(ReceivedMultiplayerNetworkMessage{
                                    .message = MultiplayerNetworkMessage{
                                        .type = MultiplayerNetworkMessageType::CONNECTED_TO_SERVER,
                                        .content = {}
                                    },
                                    .origin = message
                                });
                                break;
                            }
                            case ReceivedNetworkMessageType::DISCONNECTED_FROM_SERVER: {
                                onMessageReceived.trigger(ReceivedMultiplayerNetworkMessage{
                                    .message = MultiplayerNetworkMessage{
                                        .type = MultiplayerNetworkMessageType::DISCONNECTED_FROM_SERVER,
                                        .content = {}
                                    },
                                    .origin = message
                                });
                                break;
                            }
                            case ReceivedNetworkMessageType::PEER_CONNECTED: {
                                onMessageReceived.trigger(ReceivedMultiplayerNetworkMessage{
                                    .message = MultiplayerNetworkMessage{
                                        .type = MultiplayerNetworkMessageType::PLAYER_CONNECTED,
                                        .content = {}
                                    },
                                    .origin = message
                                });
                                break;
                            }
                            case ReceivedNetworkMessageType::PEER_DISCONNECTED: {
                                onMessageReceived.trigger(ReceivedMultiplayerNetworkMessage{
                                    .message = MultiplayerNetworkMessage{
                                        .type = MultiplayerNetworkMessageType::PLAYER_LEFT,
                                        .content = {}
                                    },
                                    .origin = message
                                });
                                break;
                            }
                            case ReceivedNetworkMessageType::NEW_MESSAGE: {
                                auto mm = MultiplayerNetworkMessage::Deserialize(message.content);

                                if (!mm.has_value()) {
                                    // TODO: error
                                    return;
                                }

                                onMessageReceived.trigger(ReceivedMultiplayerNetworkMessage{
                                    .message = mm.value(),
                                    .origin = message
                                });
                            }
                        }
                    },
                    onMessageReceived.id
                )
            );
            this->listenersIds.push_back(newId);
            return newId;
        }

        void OffMessageReceived(int id) {
            this->listenersIds.erase(
                std::remove(
                    this->listenersIds.begin(),
                    this->listenersIds.end(),
                    id
                ),
                this->listenersIds.end()
            );

            this->udpTransport->OffMessageReceived(id);
        }

        bool SendMessage(
            MultiplayerNetworkMessage message,
            ENetPacketFlag flags = ENET_PACKET_FLAG_RELIABLE,
            ENetPeer* peer = nullptr
        ) {
            return this->udpTransport->SendMessage(
                message.Serialize(),
                flags,
                peer
            );
        }

        std::optional<MultiplayerNetworkMessage> PollNextMessage(enet_uint32 customTimeout) {
            auto nextMessage = this->udpTransport->PollNextMessage(customTimeout);

            if (!nextMessage.has_value()) {
                return std::nullopt;
            }

            return MultiplayerNetworkMessage::Deserialize(nextMessage->content);
        }
};

struct WrappedOnMultiplayerMessageReceivedListener {
    cen::MultiplayerNetworkTransport* transport;
    int listenerId;

    WrappedOnMultiplayerMessageReceivedListener(
        cen::MultiplayerNetworkTransport* transport,
        std::function<void(ReceivedMultiplayerNetworkMessage)> onMessageReceived,
        int id = 0
    ): transport(transport) {
        this->listenerId = transport->OnMessageReceived(
            onMessageReceived
        );
    }

    ~WrappedOnMultiplayerMessageReceivedListener() {
        transport->OffMessageReceived(listenerId);
    }
};

}

#endif // CEN_MULTIPLAYER_H