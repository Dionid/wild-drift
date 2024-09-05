#ifndef CSP_MATCH_H_
#define CSP_MATCH_H_

#include "cengine/cengine.h"
#include "entity.h"

// class Match {
//     public:
//         int screenWidth;
//         int screenHeight;
//         Scene* scene;

//         Match(
//             const int screenWidth,
//             const int screenHeight,
//             Scene* scene
//         );
//         void InitMatch(); 
// };

class MatchManager: public Node {
    public:
        node_id_t ballId;
        node_id_t playerId;
        node_id_t enemyId;
        int playerScore;
        int enemyScore;

        MatchManager(
            int playerScore = 0,
            int enemyScore = 0
        );

        void Reset(GameContext* ctx);

        void PlayerScored();
        void EnemyScored();

        void Init(GameContext* ctx) override;
        void Update(GameContext* ctx) override;
        void Render(GameContext* ctx) override;
};

#endif // CSP_MATCH_H_