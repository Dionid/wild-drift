// #include <fstream>
// #include <nlohmann/json.hpp>
#include "entity.h"
#include "utils.h"

// using json = nlohmann::json;

// # Camera

WDCamera::WDCamera(
    Map* map,
    Player* player
): cen::Node() {
    this->map = map;
    this->player = player;
}

const cen::type_id_t WDCamera::_tid = cen::TypeIdGenerator::getInstance().getNextId();

void WDCamera::Init() {
    this->scene->camera->target = Vector2{
        (float)map->tileMap->widthInPixels / 2,
        (float)map->tileMap->heightInPixels / 2
    };

    this->scene->camera->offset = Vector2{
        (float)this->scene->screen.width / 2,
        (float)this->scene->screen.height / 2
    };
}

void WDCamera::Update() {
    auto& camera = this->scene->camera;
    const auto& screen = this->scene->screen;

    // # Move by dragging
    // if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
    //     Vector2 delta = GetMouseDelta();
    //     delta = Vector2Scale(delta, -1.0f/camera->zoom);

    //     Vector2 newTarget = Vector2Add(camera->target, delta);

    //     camera->target = newTarget;
    // }

    // # Move by keyboard
    if (IsKeyDown(KEY_RIGHT)) {
        camera->target.x += 2;
    }

    if (IsKeyDown(KEY_LEFT)) {
        camera->target.x -= 2;
    }

    if (IsKeyDown(KEY_UP)) {
        camera->target.y -= 2;
    }

    if (IsKeyDown(KEY_DOWN)) {
        camera->target.y += 2;
    }

    // # Move by following player
    camera->target = Vector2Lerp(camera->target, player->position, 0.2f);

    // # Apply bounds
    if (camera->target.x < camera->offset.x / 2) {
        camera->target.x = camera->offset.x / 2;
    } else if (camera->target.x > map->tileMap->widthInPixels - camera->offset.x / 2) {
        camera->target.x = map->tileMap->widthInPixels - camera->offset.x / 2;
    }

    if (camera->target.y < camera->offset.y / 2) {
        camera->target.y = camera->offset.y / 2;
    } else if (camera->target.y > map->tileMap->heightInPixels - camera->offset.y / 2) {
        camera->target.y = map->tileMap->heightInPixels - camera->offset.y / 2;
    }
}

// # Map
Map::Map(
    std::string name,
    std::string path,
    cen::TextureStorage* textureStorage,
    Vector2 position,
    int zOrder,
    uint16_t id,
    Node* parent
): Node2D(position, false, zOrder, id, parent) {
    this->name = name;
    this->path = path;
    this->textureStorage = textureStorage;

    this->tileMap = std::make_unique<cen::TileMap>(
        this->path
    );

    this->tileMap->Load();

    this->width = tileMap->widthInPixels;
    this->height = tileMap->heightInPixels;
}

const cen::type_id_t Map::_tid = cen::TypeIdGenerator::getInstance().getNextId();

void Map::Init() {
    this->AddNode(
        std::make_unique<cen::TileMapView>(
            tileMap.get(),
            this->textureStorage
        )
    );
}

// # Player

Player::Player(
    uint64_t playerId,
    std::string name,
    Vector2 position,
    cen::Size size,
    float speed,
    float maxVelocity,
    Map* map
): cen::CharacterBody2D(
    position,
    size,
    Vector2{},
    cen::MotionMode::Floating,
    cen::Vector2Up,
    0.1f,
    true,
    20
) {
    this->playerId = playerId;
    this->name = name;
    this->speed = speed;
    this->maxVelocity = maxVelocity;
    this->map = map;
}

const cen::type_id_t Player::_tid = cen::TypeIdGenerator::getInstance().getNextId();

void Player::Init() {
    view = this->AddNode(
        std::make_unique<cen::RectangleView>(
            this->size,
            WHITE,
            1.0f,
            Vector2{},
            true,
            this->zOrder
        )
    );

    this->AddNode(
        std::make_unique<cen::Collider>(
            cen::ColliderType::Solid,
            cen::Shape::Rectangle(this->size)
        )
    );
}

void Player::Update() {
    const auto& currentPlayerInput = this->scene->playerInputManager.playerInputs[this->playerId];

    directionY = currentPlayerInput.down - currentPlayerInput.up;
    directionX = currentPlayerInput.right - currentPlayerInput.left;
}

void Player::ApplyFriction() {
    this->velocity.y *= .60f;
    this->velocity.x *= .60f;

    if (this->velocity.x < 0.01 && this->velocity.x > -0.01) {
        this->velocity.x = 0;
    }

    if (this->velocity.y < 0.01 && this->velocity.y > -0.01) {
        this->velocity.y = 0;
    }
}

void Player::FixedUpdate() {
    Vector2 newSpeed = Vector2Scale(
        Vector2Normalize({
            this->speed * directionX,
            this->speed * directionY,
        }),
        this->speed
    );

    this->velocity.y += newSpeed.y;
    this->velocity.x += newSpeed.x;
    if (Vector2Length(this->velocity) > this->maxVelocity) {
        this->velocity = Vector2Scale(Vector2Normalize(this->velocity), this->maxVelocity);
    }

    // # Friction
    this->ApplyFriction();

    // # Map Boundaries
    // this->ApplyMapBoundaries(this->scene->screen.width, this->scene->screen.height);

    // # Velocity -> Position
    this->MoveAndSlide();

    this->view->zOrder = this->position.y + this->size.height / 2;
}