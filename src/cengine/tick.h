
#ifndef CEN_TICK_STATE_H
#define CEN_TICK_STATE_H

#include <iostream>
#include <vector>
#include "core.h"

// # Save on Client + Compute on Server

namespace cen {

typedef uint64_t tick_id_t;

// # GameStateTick
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

// # PlayerInputTick
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

        std::vector<T> pendingGameStateTicks;
        std::vector<T> arrivedGameStateTicks;
        std::vector<PlayerInputTick> playerInputTicks;

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
            if (this->arrivedGameStateTicks.size() > 100) {
                this->arrivedGameStateTicks.erase(this->arrivedGameStateTicks.begin(), this->arrivedGameStateTicks.begin() + 20);
            }

            this->arrivedGameStateTicks.push_back(gameStateTick);
        }

        void AddPendingGameState(
            const T& gameStateTick
        ) {
            static_assert(std::is_base_of<GameStateTick, T>::value, "T must be derived from GameStateTick");

            if (gameStateTick.id < this->currentTick) {
                return;
            }

            // TODO: Refactor to use a circular buffer
            if (this->pendingGameStateTicks.size() > 100) {
                this->pendingGameStateTicks.erase(this->pendingGameStateTicks.begin(), this->pendingGameStateTicks.begin() + 20);
            }

            this->pendingGameStateTicks.push_back(gameStateTick);
        }

        void AddPlayerInput(
            const PlayerInputTick& playerInputTick
        ) {
            if (playerInputTick.id < this->currentTick) {
                return;
            }

            // TODO: Refactor to use a circular buffer
            if (this->playerInputTicks.size() > 100) {
                this->playerInputTicks.erase(this->playerInputTicks.begin(), this->playerInputTicks.begin() + 20);
            }

            this->playerInputTicks.push_back(playerInputTick);
        }

        CompareResult CompareArrivedAndPending() {
            CompareResult result;

            for (int i = 0; i < this->arrivedGameStateTicks.size(); i++) {
                const T arrivedGameStateTick = this->arrivedGameStateTicks[i];
                result.found = false;
                for (int j = 0; j < this->pendingGameStateTicks.size(); j++) {
                    const T pendingGameStateTick = this->pendingGameStateTicks[j];

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

            for (int i = 0; i < this->pendingGameStateTicks.size(); i++) {
                if (this->pendingGameStateTicks[i].id == compareResult.validPendingGameStateTickId) {
                    validPendingGameStateTickIndex = i;
                    break;
                }
            }

            this->pendingGameStateTicks.erase(
                this->pendingGameStateTicks.begin(),
                this->pendingGameStateTicks.begin() + validPendingGameStateTickIndex
            );

            // # Remove validated PlayerInputTick
            int validPlayerInputTickIndex = -1;

            for (int i = 0; i < this->arrivedGameStateTicks.size(); i++) {
                if (this->playerInputTicks[i].id == compareResult.validPendingGameStateTickId) {
                    validPlayerInputTickIndex = i;
                    break;
                }
            }

            this->playerInputTicks.erase(
                this->playerInputTicks.begin(),
                this->playerInputTicks.begin() + validPlayerInputTickIndex
            );
        }
};

}

#endif // CEN_TICK_STATE_H