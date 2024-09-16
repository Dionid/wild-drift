#include "match.h"
#include "events.h"

LaunchBallTimer::LaunchBallTimer(
    cen::node_id_t ballId,
    int duration,
    cen::node_id_t id = 0,
    cen::Node* parent = nullptr
): Timer(duration, id, parent) {
    this->ballId = ballId;
}

void LaunchBallTimer::OnTimerEnd() {
    auto ball = this->scene->nodeStorage->GetById<Ball>(this->ballId);

    if (ball == nullptr) {
        return;
    }

    float randomAngle = (this->scene->simulationTick % 100 / 100.0f) * 2 * PI;
    ball->velocity.x = cos(randomAngle) * 5;
    ball->velocity.y = sin(randomAngle) * 5;
}

// # Match Manager

MatchManager::MatchManager(
    SpcAudio* gameAudio,
    StepLockNetworkManager* stepLockNetworkManager,
    int winScore,
    int playerScore,
    int enemyScore
): cen::Node2D(Vector2{}) {
    this->gameAudio = gameAudio;
    this->stepLockNetworkManager = stepLockNetworkManager;
    this->winScore = winScore;
    this->playerScore = playerScore;
    this->enemyScore = enemyScore;
};

void MatchManager::Init() {
    // # Player
    const float sixthScreen = this->scene->screen.width/6.0f;

    Player* player = this->AddNode(
        std::make_unique<Player>(
            (Vector2){ sixthScreen, this->scene->screen.height/2.0f },
            (cen::Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.5f,
            10.0f
        )
    );

    player->zOrder = 1;
    this->playerId = player->id;

    float ballRadius = 15.0f;
    float randomAngle = (this->scene->simulationTick % 100 / 100.0f) * 2 * PI;
    Ball* ball = this->AddNode(
        std::make_unique<Ball>(
            this->gameAudio,
            ballRadius,
            (Vector2){ this->scene->screen.width/2.0f, this->scene->screen.height/2.0f },
            (cen::Size){ ballRadius*2, ballRadius*2 },
            (Vector2){ cos(randomAngle) * 6, sin(randomAngle) * 6 },
            10.0f
        )
    );

    ball->zOrder = 1;
    this->ballId = ball->id;

    Enemy* enemy = this->AddNode(
        std::make_unique<Enemy>(
            ball->id,
            (Vector2){ this->scene->screen.width - sixthScreen, this->scene->screen.height/2.0f },
            (cen::Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.5f,
            10.0f
        )
    );

    enemy->zOrder = 1;
    this->enemyId = enemy->id;

    // # Goals
    cen::Size goalSize = { 10, (float)this->scene->screen.height - 20 };

    Goal* lGoal = this->AddNode(
        std::make_unique<Goal>(
            true,
            (Vector2){ goalSize.width / 2 + 10, goalSize.height / 2 + 10 },
            goalSize
        )
    );

    lGoal->zOrder = 2;

    Goal* rGoal = this->AddNode(
        std::make_unique<Goal>(
            false,
            (Vector2){ this->scene->screen.width - goalSize.width / 2 - 5, goalSize.height / 2 + 15 },
            goalSize
        )
    );

    rGoal->zOrder = 2;

    // # Field
    this->AddNode(
        std::make_unique<cen::LineView>(
            (Vector2){ this->scene->screen.width/2.0f, 80 },
            this->scene->screen.height - 160,
            WHITE,
            0.5f
        )
    );

    this->AddNode(
        std::make_unique<cen::CircleView>(
            80,
            (Vector2){ this->scene->screen.width/2.0f, this->scene->screen.height/2.0f },
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
    auto screenWidthQuoter = this->scene->screen.width / 2 / 2;
    auto fontSize = 50;

    this->playerScoreText = this->AddNode(
        std::make_unique<cen::TextView>(
            (Vector2){
                screenWidthQuoter - fontSize / 2.0f,
                this->scene->screen.height / 2.0f - fontSize / 2.0f
            },
            "0",
            fontSize,
            ColorAlpha(WHITE, 0.5f)
        )
    );

    this->enemyScoreText = this->AddNode(
        std::make_unique<cen::TextView>(
            (Vector2){
                this->scene->screen.width / 2.0f + screenWidthQuoter - fontSize / 2.0f,
                this->scene->screen.height / 2.0f - fontSize / 2.0f
            },
            "0",
            fontSize,
            ColorAlpha(WHITE, 0.5f)
        )
    );
};

void MatchManager::InitMultiplayerMode(bool isHost) {
    auto waitingNode = this->AddNode(
        std::make_unique<cen::TextView>(
            (Vector2){ this->scene->screen.width/2.0f, this->scene->screen.height/2.0f },
            "Waiting...",
            20,
            WHITE
        )
    );

    if (isHost) {
        auto result = this->stepLockNetworkManager->networkManager->InitServer();
        if (result > 0) {
            // ...
        }
    } else {
        auto result = this->stepLockNetworkManager->networkManager->InitClient();
        if (result > 0) {
            // ...
        }
    }

    this->RemoveChild(waitingNode);

    auto initted = this->stepLockNetworkManager->InitialSync();
    if (!initted) {
        return;
    }
}

void MatchManager::ResetEntities() {
    auto ball = this->scene->nodeStorage->GetById<Ball>(this->ballId);
    auto player = this->scene->nodeStorage->GetById<Player>(this->playerId);
    auto enemy = this->scene->nodeStorage->GetById<Enemy>(this->enemyId);

    if (ball == nullptr || player == nullptr || enemy == nullptr) {
        return;
    }

    ball->position = (Vector2){ this->scene->screen.width/2.0f, this->scene->screen.height/2.0f };
    ball->previousPosition = ball->position;
    ball->velocity = (Vector2){ 0.0f, 0.0f };

    player->position = (Vector2){ this->scene->screen.width/6.0f, this->scene->screen.height/2.0f };
    player->previousPosition = player->position;
    player->velocity = (Vector2){ 0.0f, 0.0f };

    enemy->position = (Vector2){ this->scene->screen.width - this->scene->screen.width/6.0f, this->scene->screen.height/2.0f };
    enemy->previousPosition = enemy->position;
    enemy->velocity = (Vector2){ 0.0f, 0.0f };

    this->launchBallTimer->Reset();
}

void MatchManager::Reset() {
    this->playerScore = 0;
    this->enemyScore = 0;

    this->ResetEntities();

    this->playerScoreText->text = std::to_string(this->playerScore);
    this->enemyScoreText->text = std::to_string(this->enemyScore);
}

void MatchManager::PlayerScored() {
    this->playerScore++;
    this->ResetEntities();
    this->playerScoreText->text = std::to_string(this->playerScore);
}

void MatchManager::EnemyScored() {
    this->enemyScore++;
    this->ResetEntities();
    this->enemyScoreText->text = std::to_string(this->enemyScore);
}

void MatchManager::FixedUpdate() {
    for (const auto& collision: this->scene->collisionEngine->startedCollisions) {
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

        if (this->playerScore >= this->winScore || this->enemyScore >= this->winScore) {
            this->scene->eventBus->emit(OnMatchEndEvent());
            if (this->playerScore > this->enemyScore) {
                // TODO: SoundManager (that will do nothing on server)
                PlaySound(this->gameAudio->win);
            } else {
                // TODO: SoundManager (that will do nothing on server)
                PlaySound(this->gameAudio->lost);
            }

            return;
        }

        // TODO: SoundManager (that will do nothing on server)
        SetSoundPitch(this->gameAudio->score, GetRandomValue(80, 120) / 100.0f);
        PlaySound(this->gameAudio->score);
    }
}