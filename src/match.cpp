#include "match.h"
#include "events.h"

LaunchBallTimer::LaunchBallTimer(
    cen::node_id_t ballId,
    int duration
): Timer(
    duration,
    cen::TimerMode::FRAMES
) {
    this->ballId = ballId;
}

void LaunchBallTimer::OnTimerEnd() {
    auto ball = this->scene->nodeStorage->GetById<Ball>(this->ballId);

    if (ball == nullptr) {
        return;
    }

    float randomAngle = (this->scene->frameTick % 100 / 100.0f) * 2 * PI;
    ball->velocity.x = cos(randomAngle) * 5;
    ball->velocity.y = sin(randomAngle) * 5;
}

// # Match Manager

MatchManager::MatchManager(
    SpcAudio* gameAudio,
    bool mirror,
    int winScore,
    int playerScore,
    int enemyScore
): cen::Node2D(Vector2{}) {
    this->gameAudio = gameAudio;
    this->winScore = winScore;
    this->playerScore = playerScore;
    this->enemyScore = enemyScore;
    this->mirror = mirror;
};

void MatchManager::Init() {
    // # Player
    const float sixthScreen = this->scene->screen.width/6.0f;

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

    // # Timer
    this->launchBallTimer = this->AddNode(
        std::make_unique<LaunchBallTimer>(
            this->ballId,
            30
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

void MatchManager::ResetEntities() {
    auto ball = this->scene->nodeStorage->GetById<Ball>(this->ballId);
    auto player = this->scene->nodeStorage->GetById<Player>(this->playerId);
    auto enemy = this->scene->nodeStorage->GetById<Player>(this->enemyId);

    if (ball == nullptr || player == nullptr || enemy == nullptr) {
        return;
    }

    ball->position = (Vector2){ this->scene->screen.width/2.0f, this->scene->screen.height/2.0f };
    ball->previousPosition = ball->position;
    ball->velocity = (Vector2){ 0.0f, 0.0f };

    // TODO: Move it to Player::Reset
    player->position = (Vector2){ this->scene->screen.width/6.0f, this->scene->screen.height/2.0f };
    player->previousPosition = player->position;
    player->velocity = (Vector2){ 0.0f, 0.0f };

    // TODO: Move it to Player::Reset
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

        std::cout << "Score frame: " << this->scene->frameTick << std::endl;

        if (this->playerScore >= this->winScore || this->enemyScore >= this->winScore) {
            this->scene->eventBus.Emit(std::make_unique<MatchEndEvent>(
                this->playerScore >= this->winScore
            ));
            this->Deactivate();

            return;
        }

        // TODO: SoundManager (that will do nothing on server)
        SetSoundPitch(this->gameAudio->score, GetRandomValue(80, 120) / 100.0f);
        PlaySound(this->gameAudio->score);
    }
}