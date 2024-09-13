
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

        void SaveGameTick() override {
            GameStateTick gameStateTick = GameStateTick(
                this->currentTick
            );

            // TODO: Move capture to simulationPipeline
            // # Input
            auto input = cen::PlayerInput{
                IsKeyDown(KEY_W),
                IsKeyDown(KEY_S),
                IsKeyDown(KEY_A),
                IsKeyDown(KEY_D)
            };

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

        void Rollback(cen::CompareResult compareResult) override {
            if (compareResult.incorrectPendingGameStateTickId == -1) {
                return;
            }

            int incorrectPendingGameStateTickIndex = -1;

            for (int i = 0; i < this->pendingGameStates.size(); i++) {
                if (this->pendingGameStates[i].id == compareResult.incorrectPendingGameStateTickId) {
                    incorrectPendingGameStateTickIndex = i;
                    break;
                }
            }

            if (incorrectPendingGameStateTickIndex == -1) {
                std::cout << "Incorrect GameStateTick not found" << std::endl;
            }

            for (int i = this->pendingGameStates.size() - 1; i >= incorrectPendingGameStateTickIndex; i--) {
                const auto& incorrectPendingGameStateTick = this->pendingGameStates[i];
                // TODO: Rollback
                // ...
            }

            this->pendingGameStates.erase(
                this->pendingGameStates.begin() + incorrectPendingGameStateTickIndex,
                this->pendingGameStates.end()
            );
        }
};

#endif // SPC_GAME_TICK_MANAGER_H