#ifndef CENGINE_LOCK_STEP_SCENE_H
#define CENGINE_LOCK_STEP_SCENE_H

#include <mutex>
#include <atomic>
#include "scene.h"
#include "network.h"

namespace cen {

struct PlayerInputTick {
    cen::player_id_t playerId;
    uint64_t tick;
    cen::PlayerInput input;
};

struct PlayerInputNetworkMessage {
    cen::player_id_t playerId;
    std::vector<PlayerInputTick> inputs;

    PlayerInputNetworkMessage() {}

    PlayerInputNetworkMessage(cen::player_id_t playerId, std::vector<PlayerInputTick> inputs) {
        this->playerId = playerId;
        this->inputs = inputs;
    }

    std::vector<uint8_t> Serialize() {
        std::vector<uint8_t> buffer;

        // Serialize playerId
        cen::player_id_t playerIdCopy = this->playerId;
        uint8_t* playerIdPtr = reinterpret_cast<uint8_t*>(&playerIdCopy);
        buffer.insert(buffer.end(), playerIdPtr, playerIdPtr + sizeof(cen::player_id_t));

        // Serialize the length of the inputs vector
        uint32_t inputsLength = static_cast<uint32_t>(this->inputs.size());
        uint8_t* inputsLengthPtr = reinterpret_cast<uint8_t*>(&inputsLength);
        buffer.insert(buffer.end(), inputsLengthPtr, inputsLengthPtr + sizeof(uint32_t));

        // Serialize the inputs themselves
        for (const auto& input: this->inputs) {
            // Serialize playerId
            cen::player_id_t inputPlayerIdCopy = input.playerId;
            uint8_t* inputPlayerIdPtr = reinterpret_cast<uint8_t*>(&inputPlayerIdCopy);
            buffer.insert(buffer.end(), inputPlayerIdPtr, inputPlayerIdPtr + sizeof(cen::player_id_t));

            // Serialize tick
            uint64_t inputTickCopy = input.tick;
            uint8_t* inputTickPtr = reinterpret_cast<uint8_t*>(&inputTickCopy);
            buffer.insert(buffer.end(), inputTickPtr, inputTickPtr + sizeof(uint64_t));

            input.input.Serialize(buffer);
        }

        return buffer;
    }

    static std::optional<PlayerInputNetworkMessage> Deserialize(std::vector<uint8_t> message) {
        PlayerInputNetworkMessage playerInputMessage;

        size_t offset = 0;

        // Deserialize playerId
        if (message.size() < sizeof(cen::player_id_t)) {
            // throw std::runtime_error("Insufficient data to deserialize playerId");

            return std::nullopt;
        }
        std::memcpy(&playerInputMessage.playerId, message.data() + offset, sizeof(cen::player_id_t));
        offset += sizeof(cen::player_id_t);

        // Deserialize the length of the inputs vector
        if (message.size() < offset + sizeof(uint32_t)) {
            // throw std::runtime_error("Insufficient data to deserialize inputs length");

            return std::nullopt;
        }
        uint32_t inputsLength = 0;
        std::memcpy(&inputsLength, message.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Deserialize the inputs themselves
        for (uint32_t i = 0; i < inputsLength; i++) {
            PlayerInputTick input;

            // Deserialize playerId
            if (message.size() < offset + sizeof(cen::player_id_t)) {
                // throw std::runtime_error("Insufficient data to deserialize input playerId");

                return std::nullopt;
            }
            std::memcpy(&input.playerId, message.data() + offset, sizeof(cen::player_id_t));
            offset += sizeof(cen::player_id_t);

            // Deserialize tick
            if (message.size() < offset + sizeof(uint64_t)) {
                // throw std::runtime_error("Insufficient data to deserialize input tick");

                return std::nullopt;
            }
            std::memcpy(&input.tick, message.data() + offset, sizeof(uint64_t));
            offset += sizeof(uint64_t);

            // Deserialize PlayerInput
            if (message.size() < offset + sizeof(cen::PlayerInput)) {
                // throw std::runtime_error("Insufficient data to deserialize input PlayerInput");

                return std::nullopt;
            }
            std::memcpy(&input.input, message.data() + offset, sizeof(cen::PlayerInput));
            offset += sizeof(cen::PlayerInput);

            playerInputMessage.inputs.push_back(input);
        }

        return playerInputMessage;
    }

    MultiplayerNetworkMessage ToMultiplayerNetworkMessage() {
        return MultiplayerNetworkMessage{
            .type = MultiplayerNetworkMessageType::GAME_DATA,
            .content = this->Serialize()
        };   
    }

    static std::optional<PlayerInputNetworkMessage> FromMultiplayerNetworkMessage(const MultiplayerNetworkMessage& message) {
        return PlayerInputNetworkMessage::Deserialize(message.content);
    }
};

class LockStepNetworkManager {
    public:
        std::atomic<bool> isRunning = false;

        MultiplayerNetworkTransport* transport;

        // # Playout delay (in ticks)
        uint64_t playoutDelay;
        
        // # Local player
        cen::player_id_t localPlayerId;
        // TODO: Change to ring buffer
        std::vector<PlayerInputTick> localInputsBuffer;

        std::vector<cen::player_id_t> connectedPlayers;

        std::mutex receivedInputMessagesMutex;
        // TODO: Change to ring buffer
        std::unordered_map<uint64_t, std::unordered_map<cen::player_id_t, PlayerInputTick>> receivedInputMessages;

        LockStepNetworkManager(
            MultiplayerNetworkTransport* transport,
            cen::player_id_t localPlayerId,
            std::vector<cen::player_id_t> connectedPlayers,
            uint64_t playoutDelay = 3
        ) {
            this->transport = transport;
            this->localPlayerId = localPlayerId;
            this->connectedPlayers = connectedPlayers;
            this->playoutDelay = playoutDelay;
        }

        void Init() {
            this->transport->OnMessageReceived(
                cen::OnMultiplayerMessageReceivedListener(
                    [
                        this
                    ](ReceivedMultiplayerNetworkMessage message) {
                        std::lock_guard<std::mutex> lock(this->receivedInputMessagesMutex);

                        if (
                            message.message.type == MultiplayerNetworkMessageType::PLAYER_LEFT
                            || message.message.type == MultiplayerNetworkMessageType::DISCONNECTED_FROM_SERVER
                        ) {
                            this->Stop();
                            return;
                        }

                        if (message.message.type != MultiplayerNetworkMessageType::GAME_DATA) {
                            return;
                        }

                        auto playerInputMessage = PlayerInputNetworkMessage::FromMultiplayerNetworkMessage(message.message);

                        if (!playerInputMessage.has_value()) {
                            return;
                        }

                        for (const auto& input: playerInputMessage.value().inputs) {
                            if (this->receivedInputMessages.find(input.tick) == this->receivedInputMessages.end()) {
                                this->receivedInputMessages[input.tick] = std::unordered_map<cen::player_id_t, PlayerInputTick>{};
                            }

                            this->receivedInputMessages[input.tick][input.playerId] = input;
                        }

                        // this->receivedInputMessages.push_back(playerInputMessage.value());
                        // if (this->receivedInputMessages.size() > 100) {
                        //     this->receivedInputMessages.erase(this->receivedInputMessages.begin() + 20);
                        // }
                    }
                )
            );

            this->isRunning.store(true, std::memory_order_release);
        }

        void SendTickInput() {
            auto message = PlayerInputNetworkMessage(
                this->localPlayerId,
                this->localInputsBuffer
            ).ToMultiplayerNetworkMessage();

            this->transport->SendMessage(
                message,
                ENET_PACKET_FLAG_UNSEQUENCED
            );
        }

        std::unordered_map<cen::player_id_t, PlayerInputTick> SendAndWaitForPlayersInputs(
            uint64_t tick,
            PlayerInput localPlayerInput
        ) {
            auto result = std::unordered_map<cen::player_id_t, PlayerInputTick>{};

            this->localInputsBuffer.push_back(
                PlayerInputTick{
                    this->localPlayerId,
                    tick + this->playoutDelay,
                    localPlayerInput
                }
            );

            if (this->localInputsBuffer.size() > 30) {
                this->localInputsBuffer.erase(this->localInputsBuffer.begin() + 20);
            }

            this->SendTickInput();

            while (
                this->isRunning.load(std::memory_order_acquire)
            ) {
                std::lock_guard<std::mutex> lock(this->receivedInputMessagesMutex);

                if (this->receivedInputMessages.size() == 0) {
                    continue;
                }

                if (this->receivedInputMessages.find(tick) == this->receivedInputMessages.end()) {
                    continue;
                }

                for (const auto& [playerId, message]: this->receivedInputMessages[tick]) {
                    result[playerId] = message;
                }

                break;
            }

            for (const auto& localInput: this->localInputsBuffer) {
                if (localInput.tick == tick) {
                    result[this->localPlayerId] = localInput;
                }
            }

            return result;
        }

        void FillLocalInputForDelayBuffer() {
            for (uint64_t i = 0; i < this->playoutDelay; i++) {
                this->localInputsBuffer.push_back(
                    PlayerInputTick{
                        this->localPlayerId,
                        i + 1,
                        cen::PlayerInput{}
                    }
                );
            }
        }

        void Stop() {
            this->isRunning.store(false, std::memory_order_release);
        }
};

class LockStepScene: public Scene {
    public:
        std::unique_ptr<LockStepNetworkManager> lockStepNetworkManager;

        LockStepScene(
            scene_name name,
            std::unique_ptr<LockStepNetworkManager> lockStepNetworkManager,
            cen::ScreenResolution screen,
            Camera2D* camera,
            RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus,
            int simulationFrameRate = 60,
            int simulationFixedFrameRate = 40,
            int simulationFixedFrameCyclesLimit = 10,
            cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{},
            std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
            std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>()
        ): Scene(
            name,
            screen,
            camera,
            renderingEngine,
            eventBus,
            simulationFrameRate,
            simulationFixedFrameRate,
            simulationFixedFrameCyclesLimit,
            playerInputManager,
            std::move(collisionEngine),
            std::move(nodeStorage)
        ) {
            this->lockStepNetworkManager = std::move(lockStepNetworkManager);
        }

        void Stop() override {
            this->lockStepNetworkManager->Stop();
            Scene::Stop();
        }

        void Run() override {
            this->lockStepNetworkManager->Init();

            if (!this->isInitialized) {
                // # Init scene
                this->FullInit();
            }

            float fixedTickEveryFrameTicks = static_cast<float>(this->simulationFrameRate) / this->fixedSimulationFrameRate;
            float accumulatedFixedFrame = 0.0f;

            this->lockStepNetworkManager->FillLocalInputForDelayBuffer();

            while (
                this->isAlive.load(std::memory_order_acquire)
            ) {
                // # Start
                auto frameStart = std::chrono::high_resolution_clock::now();

                // # Frame Tick
                this->frameTick++;

                // # Input
                auto localPlayerInput = cen::PlayerInput{
                    IsKeyDown(KEY_W),
                    IsKeyDown(KEY_S),
                    IsKeyDown(KEY_A),
                    IsKeyDown(KEY_D)
                };

                // # Get other player input
                auto playersInputs = this->lockStepNetworkManager->SendAndWaitForPlayersInputs(
                    this->frameTick,
                    localPlayerInput
                );

                // # Map players input
                for (const auto& [playerId, input]: playersInputs) {
                    if (playerId == this->lockStepNetworkManager->localPlayerId) {
                        this->playerInputManager.localPlayerInput = input.input;
                    }
                    this->playerInputManager.playerInputs[playerId] = input.input;
                }

                // # Init new nodes
                this->nodeStorage->InitNewNodes();

                // # Fixed update
                accumulatedFixedFrame += 1.0f;

                while (accumulatedFixedFrame >= fixedTickEveryFrameTicks) {
                    this->fixedFrameTick++;

                    // # Simulation current Tick
                    this->FixedSimulationTick();

                    // ## Correct time and cycles
                    accumulatedFixedFrame -= fixedTickEveryFrameTicks;
                }

                // # Initial
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseUpdate();
                }

                // # Flush events
                this->eventBus.Flush();

                // # Sync GameState and RendererState
                auto alpha = static_cast<double>(accumulatedFixedFrame) / fixedTickEveryFrameTicks;

                this->renderingEngine->SyncRenderBuffer(
                    this->nodeStorage.get(),
                    alpha
                );

                // # Wait till next frame
                while (std::chrono::high_resolution_clock::now() - frameStart <= simulationFrameRateInMs) {}
            }
        }
};

}

#endif