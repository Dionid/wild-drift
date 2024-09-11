#include "match.h"

LaunchBallTimer::LaunchBallTimer(
    cen::node_id_t ballId,
    int duration,
    cen::node_id_t id = 0,
    cen::Node* parent = nullptr
): Timer(duration, id, parent) {
    this->ballId = ballId;
}

void LaunchBallTimer::OnTimerEnd(cen::GameContext* ctx) {
    auto ball = ctx->scene->node_storage->GetById<Ball>(this->ballId);

    if (ball == nullptr) {
        return;
    }

    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * 2 * PI;
    ball->velocity.x = cos(randomAngle) * 5;
    ball->velocity.y = sin(randomAngle) * 5;
}

// # Match Manager

MatchManager::MatchManager(
    SpcAudio* gameAudio,
    std::function<void(cen::GameContext*)> onEnd,
    int winScore,
    int playerScore,
    int enemyScore
): cen::Node2D(Vector2{}) {
    this->gameAudio = gameAudio;
    this->onEnd = onEnd;
    this->winScore = winScore;
    this->playerScore = playerScore;
    this->enemyScore = enemyScore;
};

void MatchManager::Init(cen::GameContext* ctx) {
    // # Player
    const float sixthScreen = ctx->worldWidth/6.0f;

    Player* player = this->AddNode(
        std::make_unique<Player>(
            (Vector2){ sixthScreen, ctx->worldHeight/2.0f },
            (cen::Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            2.0f,
            10.0f
        )
    );

    player->setZOrder(1);
    this->playerId = player->id;

    float ballRadius = 15.0f;
    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * 2 * PI;
    Ball* ball = this->AddNode(
        std::make_unique<Ball>(
            this->gameAudio,
            ballRadius,
            (Vector2){ ctx->worldWidth/2.0f, ctx->worldHeight/2.0f },
            (cen::Size){ ballRadius*2, ballRadius*2 },
            (Vector2){ cos(randomAngle) * 7, sin(randomAngle) * 7 },
            10.0f
        )
    );

    ball->setZOrder(1);
    this->ballId = ball->id;

    Enemy* enemy = this->AddNode(
        std::make_unique<Enemy>(
            ball->id,
            (Vector2){ ctx->worldWidth - sixthScreen, ctx->worldHeight/2.0f },
            (cen::Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            2.0f,
            10.0f
        )
    );

    enemy->setZOrder(1);
    this->enemyId = enemy->id;

    // # Goals
    cen::Size goalSize = { 10, (float)ctx->worldHeight - 20 };

    Goal* lGoal = this->AddNode(
        std::make_unique<Goal>(
            true,
            (Vector2){ goalSize.width / 2 + 10, goalSize.height / 2 + 10 },
            goalSize
        )
    );

    lGoal->setZOrder(2);

    Goal* rGoal = this->AddNode(
        std::make_unique<Goal>(
            false,
            (Vector2){ ctx->worldWidth - goalSize.width / 2 - 5, goalSize.height / 2 + 15 },
            goalSize
        )
    );

    rGoal->setZOrder(2);

    // # Field
    this->AddNode(
        std::make_unique<cen::LineView>(
            (Vector2){ ctx->worldWidth/2.0f, 80 },
            ctx->worldHeight - 160,
            WHITE,
            0.5f
        )
    );

    this->AddNode(
        std::make_unique<cen::CircleView>(
            80,
            (Vector2){ ctx->worldWidth/2.0f, ctx->worldHeight/2.0f },
            WHITE,
            0.5f,
            false
        )
    );

    this->launchBallTimer = this->AddNode(
        std::make_unique<LaunchBallTimer>(
            this->ballId,
            500
        )
    );

    // # GUI
    auto screenWidthQuoter = ctx->worldWidth / 2 / 2;
    auto fontSize = 50;

    this->playerScoreText = this->AddNode(
        std::make_unique<cen::TextView>(
            (Vector2){
                screenWidthQuoter - fontSize / 2,
                ctx->worldHeight / 2 - fontSize / 2
            },
            "0",
            fontSize,
            ColorAlpha(WHITE, 0.5f)
        )
    );

    this->enemyScoreText = this->AddNode(
        std::make_unique<cen::TextView>(
            (Vector2){
                ctx->worldWidth / 2 + screenWidthQuoter - fontSize / 2,
                ctx->worldHeight / 2 - fontSize / 2
            },
            "0",
            fontSize,
            ColorAlpha(WHITE, 0.5f)
        )
    );
};

void MatchManager::ResetEntities(cen::GameContext* ctx) {
    auto ball = ctx->scene->node_storage->GetById<Ball>(this->ballId);
    auto player = ctx->scene->node_storage->GetById<Player>(this->playerId);
    auto enemy = ctx->scene->node_storage->GetById<Enemy>(this->enemyId);

    if (ball == nullptr || player == nullptr || enemy == nullptr) {
        return;
    }

    ball->position = (Vector2){ ctx->worldWidth/2, ctx->worldHeight/2 };
    ball->velocity = (Vector2){ 0.0f, 0.0f };

    player->position = (Vector2){ ctx->worldWidth/6, ctx->worldHeight/2 };
    player->velocity = (Vector2){ 0.0f, 0.0f };

    enemy->position = (Vector2){ ctx->worldWidth - ctx->worldWidth/6, ctx->worldHeight/2 };
    enemy->velocity = (Vector2){ 0.0f, 0.0f };

    this->launchBallTimer->Reset();
}

void MatchManager::Reset(cen::GameContext* ctx) {
    this->playerScore = 0;
    this->enemyScore = 0;

    this->ResetEntities(ctx);

    this->playerScoreText->text = std::to_string(this->playerScore);
    this->enemyScoreText->text = std::to_string(this->enemyScore);
}

void MatchManager::PlayerScored(cen::GameContext* ctx) {
    this->playerScore++;
    this->ResetEntities(ctx);
    this->playerScoreText->text = std::to_string(this->playerScore);
}

void MatchManager::EnemyScored(cen::GameContext* ctx) {
    this->enemyScore++;
    this->ResetEntities(ctx);
    this->enemyScoreText->text = std::to_string(this->enemyScore);
}

void MatchManager::Update(cen::GameContext* ctx) {
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
            this->EnemyScored(ctx);
        } else {
            this->PlayerScored(ctx);
        }

        if (this->playerScore >= this->winScore || this->enemyScore >= this->winScore) {
            this->onEnd(ctx);
            if (this->playerScore > this->enemyScore) {
                PlaySound(this->gameAudio->win);
            } else {
                PlaySound(this->gameAudio->lost);
            }

            return;
        }

        SetSoundPitch(this->gameAudio->score, GetRandomValue(80, 120) / 100.0f);
        PlaySound(this->gameAudio->score);
    }
}