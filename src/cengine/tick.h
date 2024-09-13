
#ifndef CEN_TICK_STATE_H
#define CEN_TICK_STATE_H

#include <iostream>
#include <vector>
#include "core.h"

// # Save on Client + Compute on Server

namespace cen {

typedef uint64_t tick_id_t;

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

struct GameStateTick {
    tick_id_t id;
    bool sent;

    GameStateTick(
        tick_id_t id
    ) {
        this->id = id;
        this->sent = false;
    }

    virtual bool compare(const GameStateTick& other) const = 0;
};

// # In fact, it is just player input buffer
struct PlayerInputTick {
    tick_id_t id;
    player_id_t playerId;
    PlayerInput input;
};

struct CompareResult {
    bool found;
    int validPendingGameStateTickId;
    
    int invalidPendingGameStateTickId;
    int validArrivedGameStateTickId;

    CompareResult() {
        this->found = true;
        this->validPendingGameStateTickId = -1;
        this->invalidPendingGameStateTickId = -1;
        this->validArrivedGameStateTickId = -1;
    }
};

template <class T>
class TickManager {
    public:
        tick_id_t currentTick;
        tick_id_t lastValidatedTick;
        std::vector<T> pendingGameStates;
        std::vector<T> arrivedGameStates;
        std::vector<PlayerInputTick> playerInputs;

        virtual void SaveGameTick(cen::PlayerInput& input) = 0;
        virtual void Rollback(cen::CompareResult compareResult) = 0;

        void AddArrivedGameState(
            const T& gameStateTick
        ) {
            static_assert(std::is_base_of<GameStateTick, T>::value, "T must be derived from GameStateTick");

            if (gameStateTick.id < this->lastValidatedTick) {
                return;
            }

            // TODO: Refactor to use a circular buffer
            if (this->arrivedGameStates.size() > 100) {
                this->arrivedGameStates.erase(this->arrivedGameStates.begin(), this->arrivedGameStates.begin() + 20);
            }

            this->arrivedGameStates.push_back(gameStateTick);
        }

        void AddPendingGameState(
            const T& gameStateTick
        ) {
            static_assert(std::is_base_of<GameStateTick, T>::value, "T must be derived from GameStateTick");

            if (gameStateTick.id < this->currentTick) {
                return;
            }

            // TODO: Refactor to use a circular buffer
            if (this->pendingGameStates.size() > 100) {
                this->pendingGameStates.erase(this->pendingGameStates.begin(), this->pendingGameStates.begin() + 20);
            }

            this->pendingGameStates.push_back(gameStateTick);
        }

        void AddPlayerInput(
            const PlayerInputTick& playerInputTick
        ) {
            if (playerInputTick.id < this->currentTick) {
                return;
            }

            // TODO: Refactor to use a circular buffer
            if (this->playerInputs.size() > 100) {
                this->playerInputs.erase(this->playerInputs.begin(), this->playerInputs.begin() + 20);
            }

            this->playerInputs.push_back(playerInputTick);
        }

        CompareResult CompareArrivedAndPending() {
            CompareResult result;

            for (int i = 0; i < this->arrivedGameStates.size(); i++) {
                const T arrivedGameStateTick = this->arrivedGameStates[i];
                result.found = false;
                for (int j = 0; j < this->pendingGameStates.size(); j++) {
                    const T pendingGameStateTick = this->pendingGameStates[j];

                    if (arrivedGameStateTick.id == pendingGameStateTick.id) {
                        result.found = true;

                        // # Check if arrived GameStateTick is valid
                        if (arrivedGameStateTick.compare(pendingGameStateTick)) {
                            result.validPendingGameStateTickId = pendingGameStateTick.id;
                        } else {
                            result.invalidPendingGameStateTickId = pendingGameStateTick.id;
                            result.validArrivedGameStateTickId = arrivedGameStateTick.id;

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

        void RemoveValidated(
            CompareResult compareResult
        ) {
            if (compareResult.validPendingGameStateTickId == -1) {
                return;
            }

            int validPendingGameStateTickIndex = -1;

            for (int i = 0; i < this->pendingGameStates.size(); i++) {
                if (this->pendingGameStates[i].id == compareResult.validPendingGameStateTickId) {
                    validPendingGameStateTickIndex = i;
                    break;
                }
            }

            this->pendingGameStates.erase(
                this->pendingGameStates.begin(),
                this->pendingGameStates.begin() + validPendingGameStateTickIndex
            );

            // # Remove validated PlayerInputTick
            int validPlayerInputTickIndex = -1;

            for (int i = 0; i < this->arrivedGameStates.size(); i++) {
                if (this->playerInputs[i].id == compareResult.validPendingGameStateTickId) {
                    validPlayerInputTickIndex = i;
                    break;
                }
            }

            this->playerInputs.erase(
                this->playerInputs.begin(),
                this->playerInputs.begin() + validPlayerInputTickIndex
            );
        }
};

}

#endif // CEN_TICK_STATE_H