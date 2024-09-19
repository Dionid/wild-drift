#include "scene.h"
#include "network.h"

#ifndef CENGINE_LOCK_STEP_SCENE_H
#define CENGINE_LOCK_STEP_SCENE_H

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

            // Serialize PlayerInput
            // std::vector<uint8_t> inputPlayerInputSerialized = input.input.Serialize();
            // buffer.insert(buffer.end(), inputPlayerInputSerialized.begin(), inputPlayerInputSerialized.end());

            input.input.Serialize(buffer);
        }

        return buffer;
    }

    static PlayerInputNetworkMessage Deserialize(std::vector<uint8_t> message) {
        PlayerInputNetworkMessage playerInputMessage;

        size_t offset = 0;

        // Deserialize playerId
        if (message.size() < sizeof(cen::player_id_t)) {
            throw std::runtime_error("Insufficient data to deserialize playerId");
        }
        std::memcpy(&playerInputMessage.playerId, message.data() + offset, sizeof(cen::player_id_t));
        offset += sizeof(cen::player_id_t);

        // Deserialize the length of the inputs vector
        if (message.size() < offset + sizeof(uint32_t)) {
            throw std::runtime_error("Insufficient data to deserialize inputs length");
        }
        uint32_t inputsLength = 0;
        std::memcpy(&inputsLength, message.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Deserialize the inputs themselves
        for (uint32_t i = 0; i < inputsLength; i++) {
            PlayerInputTick input;

            // Deserialize playerId
            if (message.size() < offset + sizeof(cen::player_id_t)) {
                throw std::runtime_error("Insufficient data to deserialize input playerId");
            }
            std::memcpy(&input.playerId, message.data() + offset, sizeof(cen::player_id_t));
            offset += sizeof(cen::player_id_t);

            // Deserialize tick
            if (message.size() < offset + sizeof(uint64_t)) {
                throw std::runtime_error("Insufficient data to deserialize input tick");
            }
            std::memcpy(&input.tick, message.data() + offset, sizeof(uint64_t));
            offset += sizeof(uint64_t);

            // Deserialize PlayerInput
            if (message.size() < offset + sizeof(cen::PlayerInput)) {
                throw std::runtime_error("Insufficient data to deserialize input PlayerInput");
            }
            std::memcpy(&input.input, message.data() + offset, sizeof(cen::PlayerInput));
            offset += sizeof(cen::PlayerInput);

            playerInputMessage.inputs.push_back(input);
        }

        return playerInputMessage;
    }
};

class LockStepNetworkManager {
    public:
        bool isStepLockActivated = false;

        bool isHost;
        NetworkManager* networkManager;
        UdpTransport* transport;

        cen::player_id_t currentPlayerId;
        std::vector<PlayerInputTick> localInputsBuffer;

        std::vector<cen::player_id_t> connectedPlayers;

        std::vector<PlayerInputNetworkMessage> receivedInputMessages;

        LockStepNetworkManager(
            NetworkManager* networkManager,
            bool isHost
        ) {
            this->networkManager = networkManager;
            this->isHost = isHost;
            this->transport = nullptr;
        }

        void ActivateStepLock() {
            this->isStepLockActivated = true;
        }

        void Init() {
            if (this->isHost) {
                this->currentPlayerId = 1;
                this->transport = this->networkManager->CreateAndInitUdpTransportServer(
                    "lock-step-transport",
                    "127.0.0.1",
                    1234,
                    1000,
                    [this](ReceivedNetworkMessage message) {
                        std::cout << "Message from client" << std::endl;

                        if (message.type != ReceivedNetworkMessageType::NEW_MESSAGE) {
                            return;
                        }

                        auto playerInputMessage = PlayerInputNetworkMessage::Deserialize(message.content);

                        this->receivedInputMessages.push_back(playerInputMessage);

                        // TODO: Change to ring buffer
                        if (this->receivedInputMessages.size() > 100) {
                            this->receivedInputMessages.erase(this->receivedInputMessages.begin() + 20);
                        }
                    }
                );
            } else {
                this->currentPlayerId = 2;
                this->transport = this->networkManager->CreateAndInitUdpTransportClient(
                    "lock-step-transport",
                    "127.0.0.1",
                    1234,
                    1000,
                    [this](ReceivedNetworkMessage message) {
                        std::cout << "Message from server" << std::endl;

                        if (message.type != ReceivedNetworkMessageType::NEW_MESSAGE) {
                            return;
                        }

                        auto playerInputMessage = PlayerInputNetworkMessage::Deserialize(message.content);

                        this->receivedInputMessages.push_back(playerInputMessage);

                        // TODO: Change to ring buffer
                        if (this->receivedInputMessages.size() > 100) {
                            this->receivedInputMessages.erase(this->receivedInputMessages.begin() + 20);
                        }
                    }
                );
            }

            this->transport->Init();
        }

        void SendTickInput() {
            auto message = PlayerInputNetworkMessage(
                this->currentPlayerId,
                this->localInputsBuffer
            ).Serialize();
            this->transport->SendMessage(message);
        }

        PlayerInputTick SendAndWaitForPlayersInputs(
            uint64_t tick,
            PlayerInput currentPlayerInput
        ) {
            // if (!this->isStepLockActivated) {
            //     return PlayerInputTick{0, 0, PlayerInput()};
            // }

            this->localInputsBuffer.push_back(
                PlayerInputTick{
                    this->currentPlayerId,
                    tick,
                    currentPlayerInput
                }
            );

            // TODO: Change to ring buffer
            if (this->localInputsBuffer.size() > 30) {
                this->localInputsBuffer.erase(this->localInputsBuffer.begin() + 20);
            }

            this->SendTickInput();

            while (true) {
                for (const auto& playerInputMessage: this->receivedInputMessages) {
                    for (const auto& input: playerInputMessage.inputs) {
                        if (input.tick == tick) {
                            return input;
                        }
                    }
                }
                // counter++;
                std::this_thread::yield();
            }

            return PlayerInputTick{0, 0, PlayerInput()};
        }
};

class LockStepScene: public Scene {
    public:
        bool isHost;
        NetworkManager* networkManager;
        std::unique_ptr<LockStepNetworkManager> lockStepNetworkManager;

        LockStepScene(
            bool isHost,
            NetworkManager* networkManager,
            scene_name name,
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
            this->isHost = isHost;
            this->networkManager = networkManager;
            this->lockStepNetworkManager = std::make_unique<LockStepNetworkManager>(networkManager, isHost);
        }

        void RunSimulation() {
            this->lockStepNetworkManager->Init();

            if (!this->isInitialized) {
                // # Init scene
                this->FullInit();
            }

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
                this->playerInputManager.currentPlayerInput = currentPlayerInput;

                // # Get other player input
                auto otherPlayerInput = this->lockStepNetworkManager->SendAndWaitForPlayersInputs(
                    this->frameTick,
                    currentPlayerInput
                );

                this->playerInputManager.playerInputs[otherPlayerInput.playerId] = otherPlayerInput.input;

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

                // # Wait till next frame
                while (std::chrono::high_resolution_clock::now() - frameStart <= simulationFrameRate) {}
            }
        }
};

}

#endif