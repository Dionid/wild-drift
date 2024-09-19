
#include <string>
#include "network.h"

#ifndef CEN_MULTIPLAYER_H
#define CEN_MULTIPLAYER_H

namespace cen {

enum class MultiplayerNetworkMessageType {
    // # Commands
    PLAYER_JOIN_REQUEST,
    PLAYER_JOIN_SUCCESS,
    
    // # Data
    GAME_DATA,

    // # Events
    PLAYER_LEFT,
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

struct ReceivedMultiplayerNetworkMessage {
    MultiplayerNetworkMessage message;
    ReceivedNetworkMessage origin;
};

class MultiplayerNetworkManager {
    public:
        std::atomic<bool> isRunning = false;
        cen::UdpTransport* udpTransport;
        std::function<void(ReceivedMultiplayerNetworkMessage)> onMessageReceived;

        MultiplayerNetworkManager(
            cen::UdpTransport* udpTransport,
            std::function<void(ReceivedMultiplayerNetworkMessage)> onMessageReceived
        ) {
            this->udpTransport = udpTransport;
            this->onMessageReceived = onMessageReceived;
            this->udpTransport->OnMessageReceived(
                std::make_unique<cen::OnMessageReceivedListener>(
                    [this](ReceivedNetworkMessage message) {
                        if (this->onMessageReceived != nullptr) {
                            auto mm = MultiplayerNetworkMessage::Deserialize(message.content);

                            if (!mm.has_value()) {
                                return;
                            }

                            this->onMessageReceived(ReceivedMultiplayerNetworkMessage{
                                .message = mm.value(),
                                .origin = message
                            });
                        }
                    }
                )
            );
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

}

#endif // CEN_MULTIPLAYER_H