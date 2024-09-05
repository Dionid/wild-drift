#include "match.h"

// # Match Manager

MatchManager::MatchManager(
    int playerScore,
    int enemyScore
): Node() {
    this->playerScore = playerScore;
    this->enemyScore = enemyScore;
};

void MatchManager::Init(GameContext* ctx) {
    // # Player
    const float sixthScreen = ctx->worldWidth/6.0f;

    auto player = this->AddNode(
        std::make_unique<Player>(
            (Vector2){ sixthScreen, ctx->worldHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f,
            10.0f
        )
    );

    this->playerId = player->id;

    float ballRadius = 15.0f;
    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * PI * 2;
    auto ball = this->AddNode(
        std::make_unique<Ball>(
            ballRadius,
            (Vector2){ ctx->worldWidth/2.0f, ctx->worldHeight/2.0f },
            (Size){ ballRadius*2, ballRadius*2 },
            (Vector2){ cos(randomAngle) * 5, sin(randomAngle) * 5 },
            7.0f
        )
    );

    this->ballId = ball->id;

    auto enemy = this->AddNode(
        std::make_unique<Enemy>(
            ball->id,
            (Vector2){ ctx->worldWidth - sixthScreen, ctx->worldHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f,
            10.0f
        )
    );

    this->enemyId = enemy->id;

    // # Goals
    Size goalSize = { 15, (float)ctx->worldHeight - 15 };

    this->AddNode(
        std::make_unique<Goal>(
            true,
            (Vector2){ goalSize.width / 2 + 5, goalSize.height / 2 + 15 },
            goalSize
        )
    );

    this->AddNode(
        std::make_unique<Goal>(
            false,
            (Vector2){ ctx->worldWidth - goalSize.width / 2 - 5, goalSize.height / 2 + 15 },
            goalSize
        )
    );

    // # Field
    this->AddNode(
        std::make_unique<LineView>(
            (Vector2){ ctx->worldWidth/2.0f, 80 },
            ctx->worldHeight - 160,
            WHITE,
            0.5f
        )
    );

    this->AddNode(
        std::make_unique<CircleView>(
            80,
            (Vector2){ ctx->worldWidth/2.0f, ctx->worldHeight/2.0f },
            WHITE,
            0.5f,
            false
        )
    );
};

void MatchManager::Reset(GameContext* ctx) {
    this->playerScore = 0;
    this->enemyScore = 0;

    auto ball = ctx->scene->node_storage->GetById<Ball>(this->ballId);
    auto player = ctx->scene->node_storage->GetById<Player>(this->playerId);
    auto enemy = ctx->scene->node_storage->GetById<Enemy>(this->enemyId);

    if (ball == nullptr || player == nullptr || enemy == nullptr) {
        return;
    }

    ball->position = (Vector2){ ctx->worldWidth/2, ctx->worldHeight/2 };

    player->position = (Vector2){ ctx->worldWidth/6, ctx->worldHeight/2 };
    player->velocity = (Vector2){ 0.0f, 0.0f };

    enemy->position = (Vector2){ ctx->worldWidth - ctx->worldWidth/6, ctx->worldHeight/2 };
    enemy->velocity = (Vector2){ 0.0f, 0.0f };
}

void MatchManager::PlayerScored() {
    this->playerScore++;
}

void MatchManager::EnemyScored() {
    this->enemyScore++;
}

void MatchManager::Update(GameContext* ctx) {
    for (const auto& collision: ctx->collisionEngine->startedCollisions) {
        bool predicate = (
            collision.collisionObjectA->TypeId() == Ball::_tid &&
            collision.collisionObjectB->TypeId() == Goal::_tid
        ) || (
            collision.collisionObjectA->TypeId() == Goal::_tid &&
            collision.collisionObjectB->TypeId() == Ball::_tid
        );

        if (
            !predicate
        ) {
            return;
        }

        Ball* ball;
        Goal* goal;

        if (collision.collisionObjectA->TypeId() == Ball::_tid) {
            ball = static_cast<Ball*>(collision.collisionObjectA);
            goal = static_cast<Goal*>(collision.collisionObjectB);
        } else {
            ball = static_cast<Ball*>(collision.collisionObjectB);
            goal = static_cast<Goal*>(collision.collisionObjectA);
        }

        if (goal->isLeft) {
            this->EnemyScored();
        } else {
            this->PlayerScored();
        }

        if (this->playerScore >= 2 || this->enemyScore >= 2) {
            this->Reset(ctx);
        }
    }
}

void MatchManager::Render(GameContext* ctx) {
    auto screenWidthQuoter = ctx->worldWidth / 2 / 2;
    auto fontSize = 50;

    DrawText(
        std::to_string(this->playerScore).c_str(),
        screenWidthQuoter - fontSize / 2,
        ctx->worldHeight / 2 - fontSize / 2,
        fontSize,
        ColorAlpha(WHITE, 0.5f)
    );

    DrawText(
        std::to_string(this->enemyScore).c_str(),
        ctx->worldWidth / 2 + screenWidthQuoter - fontSize / 2,
        ctx->worldHeight / 2 - fontSize / 2,
        fontSize,
        ColorAlpha(WHITE, 0.5f)
    );
};