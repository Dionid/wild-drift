
#include <map>
#include <vector>
#include "cengine/cengine.h"

#ifndef STEP_LOCK_MANAGER_H
#define STEP_LOCK_MANAGER_H

struct PlayerInputTick {
    uint64_t tick;
    cen::PlayerInput input;
};

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

struct PlayerInputNetworkMessage {
    cen::player_id_t playerId;
    PlayerInputTick input;

    PlayerInputNetworkMessage(cen::player_id_t playerId, PlayerInputTick input) {
        this->playerId = playerId;
        this->input = input;
    }

    std::string Serialize() {
        return std::to_string(this->playerId) + "|" + std::to_string(this->input.tick) + "|" + std::to_string(this->input.input.down) + "|" + std::to_string(this->input.input.up) + "|" + std::to_string(this->input.input.left) + "|" + std::to_string(this->input.input.right);
    }

    static PlayerInputNetworkMessage Deserialize(std::string message) {
        auto parts = split(message, '|');
        auto playerId = std::stoi(parts[0]);
        auto tick = std::stoull(parts[1]);
        bool down = std::stoi(parts[2]);
        bool up = std::stoi(parts[3]);
        bool left = std::stoi(parts[4]);
        bool right = std::stoi(parts[5]);

        return PlayerInputNetworkMessage(playerId, PlayerInputTick{tick, cen::PlayerInput{down, up, left, right}});
    }
};

class StepLockNetworkManager {
    public:
        uint64_t currentTick = 0;
        cen::player_id_t currentPlayerId;
        cen::NetworkManager* networkManager;
        std::vector<cen::player_id_t> playerIds;

        void InitialSync() {
            return;
        }

        std::unordered_map<cen::player_id_t, PlayerInputTick> SendAndWait(cen::PlayerInput currentInput) {
            if (this->currentPlayerId == 0) {
                return std::unordered_map<cen::player_id_t, PlayerInputTick>{};
            }

            if (this->playerIds.size() == 0 || this->playerIds.size() == 1) {
                return std::unordered_map<cen::player_id_t, PlayerInputTick>{};
            }

            auto currentInputTick = PlayerInputTick{this->currentTick, currentInput};

            // # Send
            auto message = PlayerInputNetworkMessage(this->currentPlayerId, currentInputTick);
            this->networkManager->BroadcastMessage(message.Serialize());

            // # Wait
            std::unordered_map<cen::player_id_t, PlayerInputTick> inputs;

            while (true) {
                auto messages = this->networkManager->PollNextMessage(5);

                if (messages.size() == 0) {
                    continue;
                }

                auto message = PlayerInputNetworkMessage::Deserialize(messages);

                inputs[message.playerId] = message.input;

                if (inputs.size() == this->playerIds.size()) {
                    break;
                }
            }

            return inputs;
        }
};

#endif // STEP_LOCK_MANAGER_H