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

    bool isServer;
    std::atomic<bool> isRunning = false;
    std::mutex busy;
    uint64_t serverPort;
    ENetAddress address;

    // # Server
    ENetHost* host;

    // # Client
    std::string serverHost;
    ENetPeer* serverPeer;

    // # On message received
    enet_uint32 defaultPollTimeout;
    std::function<void(ReceivedNetworkMessage)> onMessageReceived;

    UdpTransport(
        uint64_t serverPort = 1234,
        enet_uint32 defaultPollTimeout = 1000,
        std::string serverHost = ""
    ) {
        this->serverPort = serverPort;
        this->serverHost = serverHost;
        this->defaultPollTimeout = defaultPollTimeout;
    }

    ~UdpTransport() {
        if (host != NULL) {
            enet_host_destroy(host);
        }
    }

    int InitAsServer(
        uint64_t customServerPort = 0
    ) {
        if (host != NULL) {
            return EXIT_SUCCESS;
        }

        std::lock_guard<std::mutex> lock(busy);

        if (enet_initialize() != 0) {
            std::cerr << "An error occurred while initializing ENet." << std::endl;
            return EXIT_FAILURE;
        }

        address.host = ENET_HOST_ANY;
        address.port = customServerPort > 0 ? customServerPort : this->serverPort;

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

        this->isServer = true;
        this->isRunning.store(true, std::memory_order_release);

        return EXIT_SUCCESS;
    }

    int InitAsClient(
        uint64_t customServerPort = 0
    ) {
        if (host != NULL) {
            return EXIT_SUCCESS;
        }

        std::lock_guard<std::mutex> lock(busy);

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
        address.port = customServerPort > 0 ? customServerPort : this->serverPort;

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

        this->isServer = false;
        this->isRunning.store(true, std::memory_order_release);

        return EXIT_SUCCESS;
    }

    void Deinit() {
        std::lock_guard<std::mutex> lock(busy);

        this->isRunning.store(false, std::memory_order_release);
        if (host != NULL) {
            enet_host_destroy(host);
        }
    }

    bool SendMessage(
        std::vector<uint8_t> message,
        ENetPacketFlag flags = ENET_PACKET_FLAG_RELIABLE,
        ENetPeer* peer = nullptr
    ) {
        std::lock_guard<std::mutex> lock(busy);
        if (isRunning.load(std::memory_order_acquire) == false) {
            return false;
        }

        ENetPacket* packet = enet_packet_create(
            message.data(),
            message.size(),
            flags
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

    void AddOnMessageReceivedListener(
        std::function<void(ReceivedNetworkMessage)> onMessageReceived
    ) {
        // ...
    }

     // # Get message from host
    std::optional<ReceivedNetworkMessage> PollNextMessage(enet_uint32 customTimeout) {
        std::lock_guard<std::mutex> lock(busy);

        if (isRunning.load(std::memory_order_acquire) == false) {
            return std::nullopt;
        }

        ENetEvent event;

        enet_uint32 timeout = customTimeout < 0 ? this->defaultPollTimeout : customTimeout;

        while (enet_host_service(host, &event, timeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    ReceivedNetworkMessage message;

                    if (this->isServer) {
                        message = ReceivedNetworkMessage{
                            .type = ReceivedNetworkMessageType::PEER_CONNECTED
                            // .timestamp = 
                        };
                    } else {
                        message = ReceivedNetworkMessage{
                            .type = ReceivedNetworkMessageType::CONNECTED_TO_SERVER
                            // .timestamp = 
                        };
                    }

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;

                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    std::vector<uint8_t> data(event.packet->data, event.packet->data + event.packet->dataLength);

                    enet_packet_destroy(event.packet);
                    auto message = ReceivedNetworkMessage{
                        .type = ReceivedNetworkMessageType::NEW_MESSAGE,
                        .content = data,
                        .peer = event.peer
                        // .timestamp = 
                    };

                    if (this->onMessageReceived != nullptr) {
                        this->onMessageReceived(message);
                    }

                    return message;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    ReceivedNetworkMessage message;

                    if (this->isServer) {
                        message = ReceivedNetworkMessage{
                            .type = ReceivedNetworkMessageType::PEER_DISCONNECTED,
                            .peer = event.peer
                            // .timestamp = 
                        };
                    } else {
                        message = ReceivedNetworkMessage{
                            .type = ReceivedNetworkMessageType::DISCONNECTED_FROM_SERVER,
                            // .timestamp = 
                        };
                    }

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

        UdpTransport* CreateAndInitUdpTransportClient(
            std::string name,
            std::string serverHost,
            uint64_t serverPort,
            enet_uint32 pollTimeout
        ) {
            if (this->transports[name] != nullptr) {                
                return this->transports[name].get();
            }
            auto udpClient = std::make_unique<UdpTransport>(
                serverPort,
                pollTimeout < 0 ? this->defaultPollTimeout : pollTimeout,
                serverHost
            );
            // TODO: HANDLE INIT ERROR
            udpClient->InitAsServer();
            auto udpClientPtr = udpClient.get();
            this->transports[name] = std::move(udpClient);
            return udpClientPtr;
        }

        UdpTransport* CreateAndInitUdpTransportServer(
            std::string name,
            uint64_t serverPort,
            enet_uint32 pollTimeout
        ) {
            if (this->transports[name] != nullptr) {                
                return this->transports[name].get();
            }

            auto udpServer = std::make_unique<UdpTransport>(
                serverPort,
                pollTimeout < 0 ? this->defaultPollTimeout : pollTimeout,
                ""
            );
            // TODO: HANDLE INIT ERROR
            udpServer->InitAsClient();
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