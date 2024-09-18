#ifndef CEN_MULTIPLAYER_H
#define CEN_MULTIPLAYER_H

#include <map>
#include <vector>
#include <iostream>
#include <chrono>
#include <optional>
#include "enet/enet.h"
#include "node_storage.h"

namespace cen {

enum class NetworkMessageType {
    CONNECTED_TO_SERVER,
    DISCONNECTED_FROM_SERVER,

    PEER_CONNECTED,
    PEER_DISCONNECTED,

    NEW_MESSAGE
};

struct NetworkMessage {
    NetworkMessageType type;
    std::string data;
    uint64_t timestamp;
    ENetPeer* peer;
};

class UdpTransport {
    public:

    std::atomic<bool> isAlive = true;
    bool isServer;
    uint64_t serverPort;
    ENetAddress address;
    ENetHost* host;

    virtual int Init() = 0;
    virtual std::optional<NetworkMessage> PollNextMessage(enet_uint32 timeout) = 0;

    UdpTransport(
        uint64_t serverPort,
        bool isServer
    ): isServer(isServer), serverPort(serverPort) {}

    bool SendMessage(std::string message, ENetPeer* peer) {
        ENetPacket *packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        if (peer == nullptr) {
            if (!isServer) {
                std::cerr << "Cannot broadcast message from client" << std::endl;
                return false;
            }
            enet_host_broadcast(host, 0, packet);
        } else {
            enet_peer_send(peer, 0, packet);
            enet_host_flush(host);
        }

        return true;
    }
};

class UdpTransportServer: public UdpTransport {
    public:

    ~UdpTransportServer() {
        if (host != NULL) {
            enet_host_destroy(host);
        }
    }

    UdpTransportServer(uint64_t serverPort): UdpTransport(serverPort, true) {}

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

        return EXIT_SUCCESS;
    }

    // # Get message from host
    std::optional<NetworkMessage> PollNextMessage(enet_uint32 timeout = 0) override {
        ENetEvent event;

        while (enet_host_service(host, &event, timeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected!" << std::endl;
                    return NetworkMessage{
                        .type = NetworkMessageType::PEER_CONNECTED
                        // .timestamp = ,
                    };
                    break;
                case ENET_EVENT_TYPE_RECEIVE: {
                    std::string message = std::string((char*)event.packet->data, event.packet->dataLength);

                    enet_packet_destroy(event.packet);
                    return NetworkMessage{
                        .type = NetworkMessageType::NEW_MESSAGE,
                        .data = message,
                        // .timestamp = ,
                        .peer = event.peer
                    };
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from server!" << std::endl;
                    return NetworkMessage{
                        .type = NetworkMessageType::PEER_DISCONNECTED,
                        // .timestamp = ,
                        .peer = event.peer
                    };
                default: {
                    break;
                }
            }
        }

        return std::nullopt;
    }

    // # Broadcast message
    void BroadcastMessage(std::string message) {
        this->SendMessage(message, nullptr);
    }
};

class UdpTransportClient: public UdpTransport {
    public:
    ENetPeer* serverPeer;

    ~UdpTransportClient() {
        if (host != NULL) {
            enet_host_destroy(host);
        }
    }

    UdpTransportClient(uint64_t serverPort): UdpTransport(serverPort, false) {}

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

        enet_address_set_host(&address, "127.0.0.1");
        address.port = this->serverPort;

        serverPeer = enet_host_connect(host, &address, 2, 0);
        if (serverPeer == NULL) {
            std::cerr << "No available peers for initiating an ENet connection." << std::endl;
            return EXIT_FAILURE;
        }

        ENetEvent event;

        if (enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connection to server succeeded." << std::endl;
        } else {
            // TODO: RECONNECT
            std::cerr << "Connection to server failed." << std::endl;
            enet_peer_reset(serverPeer);
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    // # Get message from host
    std::optional<NetworkMessage> PollNextMessage(enet_uint32 timeout = 0) {
        ENetEvent event;

        while (enet_host_service(host, &event, timeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    // std::string message = (char*)event.packet->data;
                    std::string message = std::string((char*)event.packet->data, event.packet->dataLength);

                    enet_packet_destroy(event.packet);
                    return NetworkMessage{
                        .type = NetworkMessageType::NEW_MESSAGE,
                        .data = message,
                        // .timestamp = ,
                        .peer = event.peer
                    };
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from server!" << std::endl;
                    return NetworkMessage{
                        .type = NetworkMessageType::DISCONNECTED_FROM_SERVER
                        // .timestamp = ,
                    };
                default: {
                    break;
                }
            }
        }

        return std::nullopt;
    }

    // # Send message to server
    void SendMessageToServer(std::string message) {
        this->SendMessage(message, serverPeer);
    }
};


class NetworkManager {
    public:
        std::atomic<bool> isAlive = true;
        bool hadTransports = false;
        std::unordered_map<std::string, std::unique_ptr<UdpTransport>> transports;

        ~NetworkManager() {
            if (hadTransports) {
                enet_deinitialize();
            }
        }

        UdpTransportClient* CreateAndInitUdpTransportClient(
            std::string name,
            uint64_t serverPort
        ) {
            auto udpClient = std::make_unique<UdpTransportClient>(serverPort);
            // TODO: HANDLE INIT ERROR
            udpClient->Init();
            auto udpClientPtr = udpClient.get();
            this->transports[name] = std::move(udpClient);
            return udpClientPtr;
        }

        UdpTransportServer* CreateAndInitUdpTransportServer(
            std::string name,
            uint64_t serverPort
        ) {
            auto udpServer = std::make_unique<UdpTransportServer>(serverPort);
            // TODO: HANDLE INIT ERROR
            udpServer->Init();
            auto udpServerPtr = udpServer.get();
            this->transports[name] = std::move(udpServer);
            return udpServerPtr;
        }

        void Run() {
            while (isAlive.load(std::memory_order_acquire)) {
                for (auto& [name, transport]: transports) {
                    auto message = transport->PollNextMessage(0);
                    if (message.has_value()) {
                        std::cout << "Received message: " << message->data << std::endl;

                        switch (message->type) {
                            case NetworkMessageType::DISCONNECTED_FROM_SERVER:
                                std::cout << "Disconnected from server!" << std::endl;
                                break;
                            case NetworkMessageType::NEW_MESSAGE:
                                std::cout << "New message from peer: " << message->data << std::endl;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }

        void Destroy() {
            isAlive.store(false, std::memory_order_release);
        }
};

}

#endif // CEN_MULTIPLAYER_H