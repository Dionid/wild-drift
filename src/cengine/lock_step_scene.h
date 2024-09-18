#include "scene.h"
#include "network.h"

#ifndef CENGINE_LOCK_STEP_SCENE_H
#define CENGINE_LOCK_STEP_SCENE_H

namespace cen {

struct PlayerInputTick {
    uint64_t tick;
    cen::PlayerInput input;
};

static std::vector<std::string> split(const std::string& str, char delimiter) {
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

class LockStepNetworkManager {
    public:
        bool isHost;
        cen::player_id_t currentPlayerId;
        std::vector<cen::player_id_t> playerIds;
        std::unique_ptr<UdpTransport> transport;

        LockStepNetworkManager(bool isHost) {
            this->isHost = isHost;
        }

        void Init() {
            if (this->isHost) {
                this->transport = std::make_unique<UdpTransportServer>(1234);
            } else {
                this->transport = std::make_unique<UdpTransportClient>(1234);
            }

            this->transport->Init();
        }

        std::vector<PlayerInputTick> GetTickInputs() {
            auto messages = this->transport->PollAllNextMessages(0);
            auto playerInputTicks = std::vector<PlayerInputTick>{};

            for (const auto& message: messages) {
                if (message.type == NetworkMessageType::NEW_MESSAGE) {
                    auto playerInputMessage = PlayerInputNetworkMessage::Deserialize(message.data);
                    playerInputTicks.push_back(playerInputMessage.input);
                }
            }

            return playerInputTicks;
        }

        void SendTickInput(PlayerInputTick playerInputTick) {
            auto message = PlayerInputNetworkMessage(this->currentPlayerId, playerInputTick).Serialize();
            this->transport->SendMessage(message);
        }

        PlayerInputTick SendAndWaitForPlayersInputs(
            uint64_t tick,
            PlayerInput currentPlayerInput
        ) {
            auto playerInputTick = PlayerInputTick{tick, currentPlayerInput};

            this->SendTickInput(playerInputTick);

            bool notFound = true;

            while (notFound) {
                auto playerInputTicks = this->GetTickInputs();

                for (const auto& playerInputTick: playerInputTicks) {
                    if (playerInputTick.tick == tick) {
                        notFound = false;
                        break;
                    }
                }
            }

            return playerInputTick;
        }
};


class LockStepScene: public Scene {
    public:
        bool isHost;

        void RunSimulation() {
            if (!this->isInitialized) {
                // # Init scene
                this->FullInit();
            }

            LockStepNetworkManager lockStepNetworkManager(this->isHost);

            std::chrono::milliseconds accumulatedFixedTime(0);
            auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();

            while (this->isAlive.load(std::memory_order_acquire))    // Detect window close button or ESC key
            {
                // # Frame Tick
                this->frameTick++;

                // # Input
                auto currentPlayerInput = cen::PlayerInput{
                    IsKeyDown(KEY_W),
                    IsKeyDown(KEY_S),
                    IsKeyDown(KEY_A),
                    IsKeyDown(KEY_D)
                };

                // # GET ALL INPUTS FROM SERVER
                auto otherPlayerInput = lockStepNetworkManager.SendAndWaitForPlayersInputs(this->frameTick, currentPlayerInput);

                this->playerInputManager.currentPlayerInput = currentPlayerInput;

                // # Start
                auto frameStart = std::chrono::high_resolution_clock::now();

                // # Init new nodes
                this->nodeStorage->InitNewNodes();

                // # Fixed update
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
                lastFixedFrameTime = now;
                accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

                int fixedUpdateCycles = 0;
                while (accumulatedFixedTime >= simulationFixedFrameRate && fixedUpdateCycles < simulationFixedFrameCyclesLimit) {
                    this->fixedSimulationTick++;

                    // # Simulation current Tick
                    this->FixedSimulationTick();

                    // ## Correct time and cycles
                    accumulatedFixedTime -= simulationFixedFrameRate;
                    fixedUpdateCycles++;
                }

                // # Initial
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseUpdate();
                }

                // # Flush events
                this->eventBus.Flush();

                // # Sync GameState and RendererState
                auto alpha = static_cast<double>(accumulatedFixedTime.count()) / simulationFixedFrameRate.count();

                this->renderingEngine->SyncRenderBuffer(
                    this->nodeStorage.get(),
                    alpha
                );

                // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
                // # End (busy wait)
                while (std::chrono::high_resolution_clock::now() - frameStart <= simulationFrameRate) {}
            }
        }
};

}

#endif