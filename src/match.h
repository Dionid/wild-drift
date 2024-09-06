#ifndef CSP_MATCH_H_
#define CSP_MATCH_H_

#include <functional>
#include "cengine/cengine.h"
#include "entity.h"

class MatchManager: public Node {
    public:
        node_id_t ballId;
        node_id_t playerId;
        node_id_t enemyId;
        int playerScore;
        int enemyScore;
        int winScore;
        std::function<void()> onEnd;

        MatchManager(
            std::function<void()> onEnd,
            int winScore = 3,
            int playerScore = 0,
            int enemyScore = 0
        );

        void Reset(GameContext* ctx);

        void PlayerScored(GameContext* ctx);
        void EnemyScored(GameContext* ctx);
        void ResetEntities(GameContext* ctx);

        void Init(GameContext* ctx) override;
        void Update(GameContext* ctx) override;
        void Render(GameContext* ctx) override;
};

#endif // CSP_MATCH_H_