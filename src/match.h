#ifndef CSP_MATCH_H_
#define CSP_MATCH_H_

#include <functional>
#include "cengine/cengine.h"
#include "entity.h"
#include "audio.h"

class LaunchBallTimer: public cen::Timer {
    public:
        cen::node_id_t ballId;

        LaunchBallTimer(
            cen::node_id_t ballId,
            int duration,
            cen::node_id_t id,
            cen::Node* parent
        );

        void OnTimerEnd(cen::GameContext* ctx) override;
};

class MatchManager: public cen::Node2D {
    public:
        SpcAudio* gameAudio;
        cen::node_id_t ballId;
        cen::node_id_t playerId;
        cen::node_id_t enemyId;
        int playerScore;
        int enemyScore;
        int winScore;
        std::function<void()> onEnd;
        LaunchBallTimer* launchBallTimer;

        MatchManager(
            SpcAudio* gameAudio,
            std::function<void()> onEnd,
            int winScore = 3,
            int playerScore = 0,
            int enemyScore = 0
        );

        void Reset(cen::GameContext* ctx);

        void PlayerScored(cen::GameContext* ctx);
        void EnemyScored(cen::GameContext* ctx);
        void ResetEntities(cen::GameContext* ctx);

        void Init(cen::GameContext* ctx) override;
        void Update(cen::GameContext* ctx) override;
        void Render(cen::GameContext* ctx) override;
};

#endif // CSP_MATCH_H_