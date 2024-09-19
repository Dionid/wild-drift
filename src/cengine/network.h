#include <map>
#include <vector>
#include <iostream>
#include <chrono>
#include <optional>
#include <thread>
#include <functional>
#include "enet/enet.h"
#include "node_storage.h"

#ifndef CEN_NETWORK_H
#define CEN_NETWORK_H

namespace cen {

enum class ReceivedNetworkMessageType {
    CONNECTED_TO_SERVER,
    DISCONNECTED_FROM_SERVER,

    PEER_CONNECTED,
    PEER_DISCONNECTED,

    NEW_MESSAGE
};

struct ReceivedNetworkMessage {
    ReceivedNetworkMessageType type;
    uint64_t arrivalTimestamp;
    ENetPeer* peer;

    std::vector<uint8_t> content;
};

class UdpTransport {
    public:

    std::atomic<bool> isRunning = false;
    bool isServer;
    std::string serverHost;
    uint64_t serverPort;
    ENetAddress address;
    ENetHost* host;
    ENetPeer* serverPeer;
    enet_uint32 pollTimeout;
    std::function<void(ReceivedNetworkMessage)> onMessageReceived;

    virtual int Init() = 0;
    virtual std::optional<ReceivedNetworkMessage> PollNextMessage(enet_uint32 timeout) = 0;

    UdpTransport(
        std::string serverHost,
        uint64_t serverPort,
        bool isServer,
        enet_uint32 pollTimeout,
        std::function<void(ReceivedNetworkMessage)> onMessageReceived
    ) {
        this->serverHost = serverHost;
        this->serverPort = serverPort;
        this->isServer = isServer;
        this->pollTimeout = pollTimeout;
        this->onMessageReceived = onMessageReceived;
    }

    ~UdpTransport() {
        if (host != NULL) {
            enet_host_destroy(host);
        }
    }

    bool SendMessage(std::vector<uint8_t> message, ENetPeer* peer = nullptr) {
        if (isRunning.load(std::memory_order_acquire) == false) {
            return false;
        }

        // ENetPacket* packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);

        ENetPacket* packet = enet_packet_create(
            message.data(),
            message.size(),
            ENET_PACKET_FLAG_RELIABLE
        );

        if (this->isServer) {
            if (peer == nullptr) {
                enet_host_broadcast(host, 0, packet);
            } else {
                enet_peer_send(peer, 0, packet);
            }
        } else {
            if (peer == nullptr) {
                enet_peer_send(this->serverPeer, 0, packet);
            } else {
                enet_peer_send(peer, 0, packet);
            }
        }

        return true;
    }
};

class UdpTransportServer: public UdpTransport {
    public:

    UdpTransportServer(
        uint64_t serverPort,
        enet_uint32 pollTimeout,
        std::function<void(ReceivedNetworkMessage)> onMessageReceived
    ): UdpTransport("", serverPort, true, pollTimeout, onMessageReceived) {}

    int Init() override {
        if (host != NULL) {
            return EXIT_SUCCESS;
        }

        if (enet_initialize() != 0) {
            std::cerr << "An error occurred while initializing ENet." << std::endl;
            return EXIT_FAILURE;
        }

        address.host = ENET_HOST_ANY;
        address.port = this->serverPort;

        host = enet_host_create(
            &address /* the address to bind the server host to */,
            4        /* allow up to 32 clients and/or outgoing connections */,
            2        /* allow up to 2 channels */,
            0        /* incoming bandwidth */,
            0        /* outgoing bandwidth */
        );

        if (host == NULL) {
            std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Server started on port " << this->serverPort << std::endl;

        this->isRunning.store(true, std::memory_order_release);

        return EXIT_SUCCESS;
    }

    // # Get message from host
    std::optional<ReceivedNetworkMessage> PollNextMessage(enet_uint32 customTimeout) override {
        if (isRunning.load(std::memory_order_acquire) == false) {
            return std::nullopt;
        }

        ENetEvent event;

        enet_uint32 timeout = customTimeout < 0 ? this->pollTimeout : customTimeout;

        while (enet_host_service(host, &event, timeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    std::cout << "Client connected!" << std::endl;
                    auto message = ReceivedNetworkMessage{
                        .type = ReceivedNetworkMessageType::PEER_CONNECTED
                    };

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;

                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    // std::string data = std::string((char*)event.packet->data, event.packet->dataLength);

                    std::vector<uint8_t> data(event.packet->data, event.packet->data + event.packet->dataLength);

                    enet_packet_destroy(event.packet);
                    auto message = ReceivedNetworkMessage{
                        .type = ReceivedNetworkMessageType::NEW_MESSAGE,
                        .content = data,
                        // .timestamp = ,
                        .peer = event.peer
                    };

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    std::cout << "Client disconnected from server!" << std::endl;
                    auto message = ReceivedNetworkMessage{
                        .type = ReceivedNetworkMessageType::PEER_DISCONNECTED,
                        // .timestamp = ,
                        .peer = event.peer
                    };

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;
                }
                default: {
                    break;
                }
            }
        }

        return std::nullopt;
    }

    // # Broadcast message
    void BroadcastMessage(std::vector<uint8_t> message) {
        this->SendMessage(message, nullptr);
    }
};

class UdpTransportClient: public UdpTransport {
    public:

    UdpTransportClient(
        std::string serverHost,
        uint64_t serverPort,
        enet_uint32 pollTimeout,
        std::function<void(ReceivedNetworkMessage)> onMessageReceived
    ): UdpTransport(
        serverHost,
        serverPort,
        false,
        pollTimeout,
        onMessageReceived
    ) {}

    int Init() {
        if (host != NULL) {
            return EXIT_SUCCESS;
        }

        if (enet_initialize() != 0) {
            std::cerr << "An error occurred while initializing ENet." << std::endl;
            return EXIT_FAILURE;
        }

        host = enet_host_create(
            NULL /* create a client host */,
            1    /* only allow 1 outgoing connection */,
            2    /* allow up to 2 channels */,
            0    /* assume any amount of incoming bandwidth */,
            0    /* assume any amount of outgoing bandwidth */
        );

        if (host == NULL) {
            std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
            return EXIT_FAILURE;
        }

        enet_address_set_host(&address, this->serverHost.c_str());
        address.port = this->serverPort;

        serverPeer = enet_host_connect(host, &address, 2, 0);
        if (serverPeer == NULL) {
            std::cerr << "No available peers for initiating an ENet connection." << std::endl;
            return EXIT_FAILURE;
        }

        ENetEvent event;

        if (enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connection to server succeeded." << std::endl;

            if (this->onMessageReceived != nullptr) {
                this->onMessageReceived(ReceivedNetworkMessage{
                    .type = ReceivedNetworkMessageType::CONNECTED_TO_SERVER
                });
            }
        } else {
            // TODO: RECONNECT
            // ...
            std::cerr << "Connection to server failed." << std::endl;
            enet_peer_reset(serverPeer);
            return EXIT_FAILURE;
        }

        this->isRunning.store(true, std::memory_order_release);

        return EXIT_SUCCESS;
    }

    // # Get message from host
    std::optional<ReceivedNetworkMessage> PollNextMessage(enet_uint32 customTimeout = 0) {
        if (isRunning.load(std::memory_order_acquire) == false) {
            return std::nullopt;
        }

        ENetEvent event;

        enet_uint32 timeout = customTimeout < 0 ? this->pollTimeout : customTimeout;

        while (enet_host_service(host, &event, timeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    std::vector<uint8_t> data(event.packet->data, event.packet->data + event.packet->dataLength);
                    
                    enet_packet_destroy(event.packet);

                    auto message = ReceivedNetworkMessage{
                        .type = ReceivedNetworkMessageType::NEW_MESSAGE,
                        .content = data,
                        // .timestamp = ,
                        .peer = event.peer
                    };

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    std::cout << "Disconnected from server!" << std::endl;
                    auto message = ReceivedNetworkMessage{
                        .type = ReceivedNetworkMessageType::DISCONNECTED_FROM_SERVER
                        // .timestamp = ,
                    };

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;
                }
                default: {
                    break;
                }
            }
        }

        return std::nullopt;
    }

    // # Send message to server
    void SendMessageToServer(std::vector<uint8_t> message) {
        this->SendMessage(message, serverPeer);
    }
};


class NetworkManager {
    public:
        std::chrono::milliseconds messageReceivedRate;
        std::atomic<bool> isRunning = true;
        std::unordered_map<std::string, std::unique_ptr<UdpTransport>> transports;
        enet_uint32 defaultPollTimeout;
        std::function<void(ReceivedNetworkMessage)> onMessageReceived;

        NetworkManager(
            int messageReceivedRate = 60,
            enet_uint32 defaultPollTimeout = 0
        ) {
            if (enet_initialize() != 0) {
                std::cerr << "An error occurred while initializing ENet." << std::endl;
                return;
            }
            this->messageReceivedRate = std::chrono::milliseconds(1000 / messageReceivedRate);
            this->defaultPollTimeout = defaultPollTimeout;
        }

        ~NetworkManager() {
            enet_deinitialize();
        }

        UdpTransportClient* CreateAndInitUdpTransportClient(
            std::string name,
            std::string serverHost,
            uint64_t serverPort,
            enet_uint32 pollTimeout,
            std::function<void(ReceivedNetworkMessage)> onMessageReceived
        ) {
            if (this->transports[name] != nullptr) {
                auto client = dynamic_cast<UdpTransportClient*>(this->transports[name].get());
                
                return client;
            }

            auto udpClient = std::make_unique<UdpTransportClient>(
                serverHost,
                serverPort,
                pollTimeout < 0 ? this->defaultPollTimeout : pollTimeout,
                onMessageReceived
            );
            // TODO: HANDLE INIT ERROR
            udpClient->Init();
            auto udpClientPtr = udpClient.get();
            this->transports[name] = std::move(udpClient);
            return udpClientPtr;
        }

        UdpTransportServer* CreateAndInitUdpTransportServer(
            std::string name,
            uint64_t serverPort,
            enet_uint32 pollTimeout,
            std::function<void(ReceivedNetworkMessage)> onMessageReceived
        ) {
            if (this->transports[name] != nullptr) {
                auto server = dynamic_cast<UdpTransportServer*>(this->transports[name].get());
                
                return server;
            }

            auto udpServer = std::make_unique<UdpTransportServer>(
                serverPort,
                pollTimeout < 0 ? this->defaultPollTimeout : pollTimeout,
                onMessageReceived
            );
            // TODO: HANDLE INIT ERROR
            udpServer->Init();
            auto udpServerPtr = udpServer.get();
            this->transports[name] = std::move(udpServer);
            return udpServerPtr;
        }

        void DeleteTransport(std::string name) {
            this->transports.erase(name);
        }

        void Run() {
            while (isRunning.load(std::memory_order_acquire)) {
                auto frameStart = std::chrono::high_resolution_clock::now();
                
                for (auto& [name, transport]: transports) {
                    auto message = transport->PollNextMessage(0);
                    if (message.has_value() && this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message.value());
                    }
                }

                if (transports.size() == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                while (std::chrono::high_resolution_clock::now() - frameStart <= messageReceivedRate) {}
            }
        }

        void Stop() {
            isRunning.store(false, std::memory_order_release);
        }
};

}

#endif // CEN_NETWORK_H