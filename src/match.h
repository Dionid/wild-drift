#ifndef CSP_MATCH_H_
#define CSP_MATCH_H_

#include <functional>
#include "cengine/cengine.h"
#include "entity.h"
#include "audio.h"

class MatchManager: public Node {
    public:
        SpcAudio* gameAudio;
        node_id_t ballId;
        node_id_t playerId;
        node_id_t enemyId;
        int playerScore;
        int enemyScore;
        int winScore;
        std::function<void()> onEnd;

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