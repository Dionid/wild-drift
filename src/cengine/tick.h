
#ifndef CEN_TICK_STATE_H
#define CEN_TICK_STATE_H

#include <iostream>
#include <vector>
#include "core.h"

// # Save on Client + Compute on Server

namespace cen {

typedef uint64_t tick_id_t;
typedef node_id_t player_id_t;

struct PlayerInput {
    bool up;
    bool down;
    bool left;
    bool right;

    bool compare(const PlayerInput& other) const {
        return this->up == other.up &&
            this->down == other.down &&
            this->left == other.left &&
            this->right == other.right;
    }
};

struct GameStateTickPlayer {
    player_id_t playerId;
    PlayerInput input;
    Vector2 positionDelta;
    Vector2 velocityDelta;

    bool compare(const GameStateTickPlayer& other) const {
        return this->input.compare(other.input) &&
            this->positionDelta.x == other.positionDelta.x &&
            this->positionDelta.y == other.positionDelta.y &&
            this->velocityDelta.x == other.velocityDelta.x &&
            this->velocityDelta.y == other.velocityDelta.y;
    }

    void add(const GameStateTickPlayer& other) {
        this->positionDelta.x += other.positionDelta.x;
        this->positionDelta.y += other.positionDelta.y;
        this->velocityDelta.x += other.velocityDelta.x;
        this->velocityDelta.y += other.velocityDelta.y;
    }
};

struct GameStateTickBall {
    node_id_t ballId;
    Vector2 position;
    Vector2 velocity;

    bool compare(const GameStateTickBall& other) const {
        return this->position.x == other.position.x &&
            this->position.y == other.position.y &&
            this->velocity.x == other.velocity.x &&
            this->velocity.y == other.velocity.y;
    }

    void add(const GameStateTickBall& other) {
        this->position.x += other.position.x;
        this->position.y += other.position.y;
        this->velocity.x += other.velocity.x;
        this->velocity.y += other.velocity.y;
    }
};

struct GameStateTick {
    tick_id_t id;
    GameStateTickPlayer player1;
    GameStateTickPlayer player2;
    GameStateTickBall ball;
    // # Vector of added entities
    // ...
    // # Vector of deleted entities
    // ...

    bool compare(const GameStateTick& other) const {
        return this->id == other.id &&
            this->player1.compare(other.player1) &&
            this->player2.compare(other.player2) &&
            this->ball.compare(other.ball);
    }

    void add(const GameStateTick& other) {
        this->player1.add(other.player1);
        this->player2.add(other.player2);
        this->ball.add(other.ball);
    }
};

// # In fact, it is just player input buffer
class PlayerInputTick {
    tick_id_t id;
    player_id_t playerId;
    PlayerInput input;
};

struct CompareResult {
    bool found;
    int correctPendingGameStateTickId;
    int incorrectPendingGameStateTickId;
    int correctArrivedGameStateTickId;

    CompareResult() {
        this->found = true;
        this->correctPendingGameStateTickId = -1;
        this->incorrectPendingGameStateTickId = -1;
        this->correctArrivedGameStateTickId = -1;
    }
};

class TickManager {
    public:
        int currentTick;
        std::vector<GameStateTick> pendingGameStates;
        std::vector<GameStateTick> arrivedGameStates;
        std::vector<PlayerInputTick> playerInputs;

        CompareResult CompareArrivedAndPending() {
            CompareResult result;

            for (int i = 0; i < this->arrivedGameStates.size(); i++) {
                const GameStateTick& arrivedGameStateTick = this->arrivedGameStates[i];
                result.found = false;
                for (int j = 0; j < this->pendingGameStates.size(); j++) {
                    const GameStateTick& pendingGameStateTick = this->pendingGameStates[j];

                    if (arrivedGameStateTick.id == pendingGameStateTick.id) {
                        result.found = true;

                        // # Check if arrived GameStateTick is correct
                        if (arrivedGameStateTick.compare(pendingGameStateTick)) {
                            result.correctPendingGameStateTickId = pendingGameStateTick.id;
                        } else {
                            result.incorrectPendingGameStateTickId = pendingGameStateTick.id;
                            result.correctArrivedGameStateTickId = arrivedGameStateTick.id;

                            return result;
                        }
                    }
                }
            }

            if (!result.found) {
                std::cout << "GameStateTick not found" << std::endl;
            }

            return result;
        }

        void MergeCorrectGameStateTick(
            CompareResult compareResult
        ) {
            if (compareResult.correctPendingGameStateTickId == -1) {
                return;
            }

            int correctPendingGameStateTickIndex = -1;

            for (int i = 0; i < this->pendingGameStates.size(); i++) {
                if (this->pendingGameStates[i].id == compareResult.correctPendingGameStateTickId) {
                    correctPendingGameStateTickIndex = i;
                    break;
                }
            }

            // # Merge correct GameStateTick
            for (int i = 0; i < correctPendingGameStateTickIndex; i++) {
                if (i + 1 < correctPendingGameStateTickIndex) {
                    auto& gameStateTick = this->pendingGameStates[i];
                    auto& nextGameStateTick = this->pendingGameStates[i + 1];

                    nextGameStateTick.add(gameStateTick);
                }
            }

            // # Remove merged GameStateTicks
            this->pendingGameStates.erase(
                this->pendingGameStates.begin(),
                this->pendingGameStates.begin() + correctPendingGameStateTickIndex
            );
        }
};

}

#endif // CEN_TICK_STATE_H