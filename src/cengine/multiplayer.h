#ifndef CEN_MULTIPLAYER_H
#define CEN_MULTIPLAYER_H

#include <map>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "raylib.h"
#include "game_context.h"
#include "node_storage.h"

namespace cen {

typedef uint64_t player_id_t;

class NetworkMessageItem {
    public:
        virtual void Serialize() = 0;
        virtual void Deserialize() = 0;
};

class PlayerInput: public NetworkMessageItem {
    public:
        bool isLeft;
        bool isRight;
        bool isTop;
        bool isDown;

        void Serialize() override {

        };

        void Deserialize() override {

        };
};

class NetworkMessage: public NetworkMessageItem {
    public:
        cen::player_id_t playerId;
        std::vector<NetworkMessageItem> items;

    void Serialize() override {

    };

    void Deserialize() override {

    };
};

class MultiplayerManager {
    public:
        std::unordered_map<int, PlayerInput> otherPlayersInputStorage;
        cen::NodeStorage* nodeStorage;
    
    int runPipeline() {
        std::chrono::milliseconds targetSyncTime(1000 / 60);

        while (!WindowShouldClose())    // Detect window close button or ESC key
        {
            // # Start
            auto tickStart = std::chrono::high_resolution_clock::now();

            // # Receive messages
            // ...

            // # Send Messages
            // ...
            
            // # End (busy wait)
            while (std::chrono::high_resolution_clock::now() - tickStart <= targetSyncTime) {}
        }

        return EXIT_SUCCESS;
    }
};

}

#endif // CEN_MULTIPLAYER_H