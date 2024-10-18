#include <fstream>
#include <nlohmann/json.hpp>
#include "entity.h"
#include "utils.h"

using json = nlohmann::json;

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
    camera->target = Vector2Lerp(camera->target, player->position, 0.1f);

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
): Node2D(position, zOrder, id, parent) {
    this->name = name;
    this->path = path;
    this->textureStorage = textureStorage;

    std::string jsonPath = "map/" + this->path + ".json";
    std::ifstream f(cen::GetResourcePath(jsonPath));
    json data = json::parse(f);

    this->tileMap = std::make_unique<cen::TileMap>();

    tileMap->path = this->path;
    tileMap->width = data["width"];
    tileMap->height = data["height"];
    tileMap->tileWidth = data["tilewidth"];
    tileMap->tileHeight = data["tileheight"];
    tileMap->orientation = data["orientation"];
    tileMap->renderOrder = data["renderorder"];

    tileMap->widthInPixels = tileMap->width * tileMap->tileWidth;
    tileMap->heightInPixels = tileMap->height * tileMap->tileHeight;

    for (auto& tileSet : data["tilesets"]) {
        auto ts = std::make_unique<cen::TileSet>();

        ts->name = tileSet["name"];
        ts->imagePath = tileSet["image"];
        ts->columns = tileSet["columns"];
        ts->tileCount = tileSet["tilecount"];
        ts->tileHeight = tileSet["tileheight"];
        ts->imageHeight = tileSet["imageheight"];
        ts->imageWidth = tileSet["imagewidth"];
        ts->firstGID = tileSet["firstgid"];
        ts->lastGID = ts->firstGID + ts->tileCount - 1;

        tileMap->tileSets[ts->name] = std::move(ts);
    }

    for (auto& layer : data["layers"]) {
        if (layer["type"] == "tilelayer") {
            cen::TileMapLayer tileMapLayer;

            tileMapLayer.id = layer["id"];
            tileMapLayer.name = layer["name"];
            tileMapLayer.width = layer["width"];
            tileMapLayer.height = layer["height"];
            tileMapLayer.visible = layer["visible"];
            tileMapLayer.type = layer["type"];
            tileMapLayer.opacity = layer["opacity"];

            for (auto& tile : layer["data"]) {
                tileMapLayer.data.push_back(tile);
            }

            tileMap->layers.push_back(tileMapLayer);
        }
    }
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

void Map::Update() {
    
}

// # Player

Player::Player(
    uint64_t playerId,
    std::string name,
    Vector2 position,
    cen::Size size,
    float speed,
    float maxVelocity
): cen::CharacterBody2D(
    position,
    size
) {
    this->playerId = playerId;
    this->name = name;
    this->speed = speed;
    this->maxVelocity = maxVelocity;

    std::cout << "Player " << this->id << std::endl;
}

const cen::type_id_t Player::_tid = cen::TypeIdGenerator::getInstance().getNextId();

void Player::Init() {
    this->AddNode(
        std::make_unique<cen::RectangleView>(
            this->size
        )
    );
}

void Player::Update() {
    const auto& currentPlayerInput = this->scene->playerInputManager.playerInputs[this->playerId];

    directionY = currentPlayerInput.down - currentPlayerInput.up;
    directionX = currentPlayerInput.right - currentPlayerInput.left;
}

void Player::ApplyFriction() {
    this->velocity.y *= .80f;
    this->velocity.x *= .80f;

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
    this->ApplyVelocityToPosition();
}