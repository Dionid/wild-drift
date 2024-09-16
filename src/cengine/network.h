#ifndef CEN_MULTIPLAYER_H
#define CEN_MULTIPLAYER_H

#include <map>
#include <vector>
#include <iostream>
#include "enet/enet.h"
#include "node_storage.h"

namespace cen {

class NetworkNode {
    public:
        NetworkNode() {}

        virtual void OnSpawn() = 0;
};

class NetworkManager {
    public:

    bool isServer;
    uint64_t serverPort;
    ENetAddress address;
    ENetHost* server;
    ENetHost* client;
    ENetPeer* peer;
    std::chrono::milliseconds targetSyncTime;

    ~NetworkManager() {
        enet_deinitialize();
        enet_host_destroy(server);
    }

    NetworkManager(uint64_t serverPort) {
        this->serverPort = serverPort;
        targetSyncTime = std::chrono::milliseconds(1000 / 60);
    }

    int InitClient() {
        if (client != NULL) {
            return EXIT_SUCCESS;
        }

        if (enet_initialize() != 0) {
            std::cerr << "An error occurred while initializing ENet." << std::endl;
            return EXIT_FAILURE;
        }
        atexit(enet_deinitialize);

        client = enet_host_create(
            NULL /* create a client host */,
            1    /* only allow 1 outgoing connection */,
            2    /* allow up to 2 channels */,
            0    /* assume any amount of incoming bandwidth */,
            0    /* assume any amount of outgoing bandwidth */
        );

        if (client == NULL) {
            std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
            return EXIT_FAILURE;
        }

        enet_address_set_host(&address, "127.0.0.1");
        address.port = this->serverPort;

        peer = enet_host_connect(client, &address, 2, 0);
        if (peer == NULL) {
            std::cerr << "No available peers for initiating an ENet connection." << std::endl;
            return EXIT_FAILURE;
        }

        ENetEvent event;

        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connection to server succeeded." << std::endl;

            // Send a packet to the server
            // const char *message = "Hello from client!";
            // ENetPacket *packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
            // enet_peer_send(peer, 0, packet);
            // enet_host_flush(client);
        } else {
            std::cerr << "Connection to server failed." << std::endl;
            enet_peer_reset(peer);
            return EXIT_FAILURE;
        }

        this->isServer = false;

        return EXIT_SUCCESS;
    }

    int InitServer() {
        if (server != NULL || this->client != NULL) {
            return EXIT_SUCCESS;
        }

        if (enet_initialize() != 0) {
            std::cerr << "An error occurred while initializing ENet." << std::endl;
            return EXIT_FAILURE;
        }

        address.host = ENET_HOST_ANY;
        address.port = this->serverPort;

        server = enet_host_create(
            &address /* the address to bind the server host to */,
            4        /* allow up to 32 clients and/or outgoing connections */,
            2        /* allow up to 2 channels */,
            0        /* incoming bandwidth */,
            0        /* outgoing bandwidth */
        );

        if (server == NULL) {
            std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Server started on port " << this->serverPort << std::endl;

        this->isServer = true;

        return EXIT_SUCCESS;
    }

    // # Get message from client
    std::string PollNextMessage(enet_uint32 timeout = 0) {
        ENetEvent event;

        auto host = this->isServer ? server : client;

        while (enet_host_service(host, &event, timeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    std::string message = (char*)event.packet->data;
                    enet_packet_destroy(event.packet);
                    return message;
                }
                default: {
                    break;
                }
            }
        }

        return "";
    }

    // # Broadcast message
    void BroadcastMessageToClients(std::string message) {
        ENetPacket *packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(server, 0, packet);
    }

    // # Send message to server
    void SendMessageToServer(std::string message) {
        ENetPacket *packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(client);
    }

    // int runServerPipeline() {
    //     ENetEvent event;

    //     while (!WindowShouldClose())    // Detect window close button or ESC key
    //     {
    //         // # Start
    //         auto tickStart = std::chrono::high_resolution_clock::now();

    //         while (enet_host_service(server, &event, 0) > 0) {
    //             switch (event.type) {
    //                 case ENET_EVENT_TYPE_CONNECT: {
    //                     std::cout << "A new client connected from "
    //                             << event.peer->address.host << ":" << event.peer->address.port << std::endl;
    //                     event.peer->data = (void*)"Client information";
    //                     break;
    //                 }

    //                 case ENET_EVENT_TYPE_RECEIVE: {
    //                     std::cout << "Packet received from client: " << (char*)event.packet->data << std::endl;
                        
    //                     // Sending a response to the client
    //                     // const char *response = "Hello from server!";
    //                     // ENetPacket *packet = enet_packet_create(response, strlen(response) + 1, ENET_PACKET_FLAG_RELIABLE);
    //                     // enet_peer_send(event.peer, 0, packet);
                        
    //                     enet_packet_destroy(event.packet);
    //                     break;
    //                 }

    //                 case ENET_EVENT_TYPE_DISCONNECT: {
    //                     std::cout << "Client disconnected." << std::endl;
    //                     event.peer->data = NULL;
    //                     break;
    //                 }

    //                 default: {
    //                     break;
    //                 }
    //             }
    //         }

    //         // # Send Messages
    //         // ...

    //         // # End (busy wait)
    //         while (std::chrono::high_resolution_clock::now() - tickStart <= targetSyncTime) {}
    //     }

    //     enet_host_destroy(server);
    //     return EXIT_SUCCESS;
    // }

    // int runClientPipeline() {
    //     std::chrono::milliseconds targetSyncTime(1000 / 60);

    //     if (enet_initialize() != 0) {
    //         std::cerr << "An error occurred while initializing ENet." << std::endl;
    //         return EXIT_FAILURE;
    //     }
    //     atexit(enet_deinitialize);

    //     ENetHost *client;
    //     ENetAddress address;
    //     ENetPeer *peer;

    //     client = enet_host_create(
    //         NULL /* create a client host */,
    //         1    /* only allow 1 outgoing connection */,
    //         2    /* allow up to 2 channels */,
    //         0    /* assume any amount of incoming bandwidth */,
    //         0    /* assume any amount of outgoing bandwidth */
    //     );

    //     if (client == NULL) {
    //         std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
    //         return EXIT_FAILURE;
    //     }

    //     enet_address_set_host(&address, "127.0.0.1");
    //     address.port = this->serverPort;

    //     peer = enet_host_connect(client, &address, 2, 0);
    //     if (peer == NULL) {
    //         std::cerr << "No available peers for initiating an ENet connection." << std::endl;
    //         return EXIT_FAILURE;
    //     }

    //     ENetEvent event;

    //     if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
    //         std::cout << "Connection to server succeeded." << std::endl;

    //         // Send a packet to the server
    //         // const char *message = "Hello from client!";
    //         // ENetPacket *packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
    //         // enet_peer_send(peer, 0, packet);
    //         // enet_host_flush(client);
    //     } else {
    //         std::cerr << "Connection to server failed." << std::endl;
    //         enet_peer_reset(peer);
    //         return EXIT_FAILURE;
    //     }

    //     while (!WindowShouldClose())    // Detect window close button or ESC key
    //     {
    //         // # Start
    //         auto tickStart = std::chrono::high_resolution_clock::now();

    //         while (enet_host_service(client, &event, 1000) > 0) {
    //             switch (event.type) {
    //                 case ENET_EVENT_TYPE_RECEIVE:
    //                     std::cout << "Packet received from server: " << (char*)event.packet->data << std::endl;
    //                     enet_packet_destroy(event.packet);
    //                     break;

    //                 case ENET_EVENT_TYPE_DISCONNECT:
    //                     std::cout << "Disconnected from server." << std::endl;
    //                     return EXIT_SUCCESS;

    //                 default:
    //                     break;
    //             }
    //         }

    //         // # Send Messages
    //         // ...

    //         // # End (busy wait)
    //         while (std::chrono::high_resolution_clock::now() - tickStart <= targetSyncTime) {}
    //     }

    //     enet_host_destroy(client);
    //     return EXIT_SUCCESS;
    // }
};

}

#endif // CEN_MULTIPLAYER_H