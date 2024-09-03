#include "main_level.h"

MainLevel::MainLevel(
    const int screenWidth,
    const int screenHeight,
    Scene* scene
) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    this->scene = scene;
}

void MainLevel::InitMainLevel() {
    // # Player
    const float sixthScreen = this->screenWidth/6.0f;

    auto player = this->scene->node_storage->AddNode(
        Player::NewPlayer(
            (Vector2){ sixthScreen, this->screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f,
            10.0f
        )
    );

    float ballRadius = 15.0f;
    float randomAngle = (GetRandomValue(0, 100) / 100.0f) * PI * 2;
    auto ball = this->scene->node_storage->AddNode(
        Ball::NewBall(
            ballRadius,
            this->screenWidth,
            this->screenHeight,
            10.0f
        )
    );

    auto enemy = this->scene->node_storage->AddNode(
        Enemy::NewEnemy(
            ball->id,
            (Vector2){ this->screenWidth - sixthScreen, this->screenHeight/2.0f },
            (Size){ 40.0f, 120.0f },
            (Vector2){ 0.0f, 0.0f },
            1.0f,
            10.0f
        )
    );

    // # Goals
    Size goalSize = { 15, (float)this->screenHeight - 15 };

    auto leftGoal = Goal::NewGoal(
        true,
        (Vector2){ goalSize.width / 2 + 5, goalSize.height / 2 + 15 },
        goalSize
    );

    this->scene->node_storage->AddNode(std::move(leftGoal));

    auto rightGoal = Goal::NewGoal(
        false,
        (Vector2){ this->screenWidth - goalSize.width / 2 - 5, goalSize.height / 2 + 15 },
        goalSize
    );

    this->scene->node_storage->AddNode(std::move(rightGoal));

    // # Field
    this->scene->node_storage->AddNode(
        std::make_unique<LineView>(
            (Vector2){ this->screenWidth/2.0f, 80 },
            this->screenHeight - 160,
            WHITE,
            0.5f
        )
    );

    this->scene->node_storage->AddNode(
        std::make_unique<CircleView>(
            80,
            (Vector2){ this->screenWidth/2.0f, this->screenHeight/2.0f },
            WHITE,
            0.5f,
            false
        )
    );

    // # Score Manager
    this->scene->node_storage->AddNode(
        std::make_unique<LevelManager>(
            ball->id,
            player->id,
            enemy->id,
            0,
            0
        )
    );
}