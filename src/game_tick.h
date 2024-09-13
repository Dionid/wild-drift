
#include "cengine/cengine.h"
#include "entity.h"

#ifndef SPC_GAME_TICK_MANAGER_H
#define SPC_GAME_TICK_MANAGER_H

struct GameStateTickPlayer {
    cen::player_id_t playerId;
    cen::PlayerInput input;
    Vector2 position;
    Vector2 velocity;

    bool compare(const GameStateTickPlayer& other) const {
        return this->input.compare(other.input) &&
            this->position.x == other.position.x &&
            this->position.y == other.position.y &&
            this->velocity.x == other.velocity.x &&
            this->velocity.y == other.velocity.y;
    }
};

struct GameStateTickEnemy {
    cen::player_id_t playerId;
    Vector2 position;
    Vector2 velocity;

    bool compare(const GameStateTickEnemy& other) const {
        return this->position.x == other.position.x &&
            this->position.y == other.position.y &&
            this->velocity.x == other.velocity.x &&
            this->velocity.y == other.velocity.y;
    }
};

struct GameStateTickBall {
    cen::node_id_t ballId;
    Vector2 position;
    Vector2 velocity;

    bool compare(const GameStateTickBall& other) const {
        return this->position.x == other.position.x &&
            this->position.y == other.position.y &&
            this->velocity.x == other.velocity.x &&
            this->velocity.y == other.velocity.y;
    }
};

struct GameStateTick: public cen::GameStateTick {
    GameStateTickPlayer player;
    GameStateTickEnemy enemy;
    GameStateTickBall ball;

    GameStateTick(
        cen::tick_id_t id
    ): cen::GameStateTick(id) {}

    // # Vector of added entities
    // ...
    // # Vector of deleted entities
    // ...

    bool compare(const cen::GameStateTick& rawOther) const override {
        auto other = static_cast<const GameStateTick&>(rawOther);

        return this->id == other.id &&
            this->player.compare(other.player) &&
            this->enemy.compare(other.enemy) &&
            this->ball.compare(other.ball);
    }
};

class SpcGameTickManager: public cen::TickManager<GameStateTick> {
    public:
        cen::NodeStorage* nodeStorage;
        cen::node_id_t ballId;
        cen::node_id_t playerId;
        cen::node_id_t enemyId;

        SpcGameTickManager(cen::NodeStorage* nodeStorage): cen::TickManager<GameStateTick>() {
            this->nodeStorage = nodeStorage;
        }

        void SaveGameTick(
            cen::PlayerInput& input
        ) override {
            GameStateTick gameStateTick = GameStateTick(
                this->currentTick
            );

            // # Add PlayerInputTick
            this->AddPlayerInput(cen::PlayerInputTick{
                this->currentTick,
                this->playerId,
                input
            });

            // # Player
            auto player = this->nodeStorage->GetById<Player>(this->playerId);

            if (player != nullptr) {
                gameStateTick.player.playerId = player->id;
                gameStateTick.player.position = player->position;
                gameStateTick.player.velocity = player->velocity;
                gameStateTick.player.input = input;
            }

            // # Enemy
            auto enemy = this->nodeStorage->GetById<Enemy>(this->enemyId);

            if (enemy != nullptr) {
                gameStateTick.enemy.playerId = enemy->id;
                gameStateTick.enemy.position = enemy->position;
                gameStateTick.enemy.velocity = enemy->velocity;
            }

            // # Ball
            auto ball = this->nodeStorage->GetById<Ball>(this->ballId);

            if (ball != nullptr) {
                gameStateTick.ball.ballId = ball->id;
                gameStateTick.ball.position = ball->position;
                gameStateTick.ball.velocity = ball->velocity;
            }

            this->AddPendingGameState(gameStateTick);
        }

        void ApplyGameTick(GameStateTick gameStateTick) {
            // # Player
            auto player = this->nodeStorage->GetById<Player>(gameStateTick.player.playerId);

            if (player != nullptr) {
                player->position = gameStateTick.player.position;
                player->velocity = gameStateTick.player.velocity;
            }

            // # Enemy
            auto enemy = this->nodeStorage->GetById<Enemy>(gameStateTick.enemy.playerId);

            if (enemy != nullptr) {
                enemy->position = gameStateTick.enemy.position;
                enemy->velocity = gameStateTick.enemy.velocity;
            }

            // # Ball
            auto ball = this->nodeStorage->GetById<Ball>(gameStateTick.ball.ballId);

            if (ball != nullptr) {
                ball->position = gameStateTick.ball.position;
                ball->velocity = gameStateTick.ball.velocity;
            }
        }

        void Rollback(cen::CompareResult compareResult) override {
            if (compareResult.invalidPendingGameStateTickId == -1) {
                return;
            }

            int invalidPendingGameStateTickIndex = -1;

            for (int i = 0; i < this->pendingGameStateTicks.size(); i++) {
                if (this->pendingGameStateTicks[i].id == compareResult.invalidPendingGameStateTickId) {
                    invalidPendingGameStateTickIndex = i;
                    break;
                }
            }

            if (invalidPendingGameStateTickIndex == -1) {
                std::cout << "Invalid GameStateTick not found" << std::endl;
                return;
            }

            for (int i = this->pendingGameStateTicks.size() - 1; i >= invalidPendingGameStateTickIndex; i--) {
                const auto& invalidPendingGameStateTick = this->pendingGameStateTicks[i];
                // TODO: Rollback events (create, add of Nodes)
                // ...
            }

            // # Empty pending GameStateTicks
            this->pendingGameStateTicks.erase(
                this->pendingGameStateTicks.begin(),
                this->pendingGameStateTicks.end()
            );

            // # Apply last arrived
            auto lastValidArrivedGameStateTick = this->arrivedGameStateTicks.back();
            this->lastValidatedTick = lastValidArrivedGameStateTick.id;
            this->ApplyGameTick(lastValidArrivedGameStateTick);

            // # Empty arrived GameStateTicks
            this->arrivedGameStateTicks.erase(
                this->arrivedGameStateTicks.begin(),
                this->arrivedGameStateTicks.end()
            );

            // # Remove PlayerInputTicks till last arrived
            auto lastValidPlayerInputTickIndex = -1;

            for (int i = 0; i < this->playerInputTicks.size(); i++) {
                if (this->playerInputTicks[i].id == lastValidArrivedGameStateTick.id) {
                    lastValidPlayerInputTickIndex = i;
                    break;
                }
            }

            if (lastValidPlayerInputTickIndex == -1) {
                std::cout << "Last valid PlayerInputTick not found" << std::endl;
                return;
            }

            this->playerInputTicks.erase(
                this->playerInputTicks.begin(),
                this->playerInputTicks.begin() + lastValidPlayerInputTickIndex
            );
            
            // // TODO: # Simulate new GameTicks using PlayerInputTicks
            // for (const auto& playerInput: this->playerInputs) {
            //     // ...
            // }
        }
};

#endif // SPC_GAME_TICK_MANAGER_H