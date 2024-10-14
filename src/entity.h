#include "cengine/cengine.h"

#ifndef CSP_ENTITY_H_
#define CSP_ENTITY_H_

// # Map

class Map: public cen::Node2D {
    public:
    std::string name;
    std::string description;
    std::string path;

    Map(
        std::string name,
        std::string description,
        std::string path,
        Vector2 position,
        int zOrder = 0,
        uint16_t id = 0,
        Node* parent = nullptr
    );
};

// // # Characters & Players

// // ## Ability

// enum AbilityTargetType {
//     Instant  = 0,        // 0000 Instant
//     Self     = 1 << 0,   // 0001 Self
//     Allies   = 1 << 1,   // 0010 Allies
//     Enemies  = 1 << 2    // 0100 Enemies
// };

// struct Ability {
//     std::string name;
//     std::string description;
//     AbilityTargetType targetType;
//     float cooldown;
// };

// struct AbilityBinding {
//     Ability ability;
//     uint8_t key;
// };

// // ## Attack

// enum class AttackType {
//     MELEE,
//     RANGED
// };

// enum class DamageApplyType {
//     PERCENTAGE,
//     FLAT
// };

// class Attack {
//     public:

//     AttackType type;
//     DamageApplyType damageApplyType;
//     float damage;
//     float range;
//     float cooldown;
//     float duration;
// };

// // ## Champion

// struct ChampionStats {
//     uint16_t health;
//     uint16_t movementSpeed;
// };

// struct Champion {
//     std::string name;
//     std::string description;
//     Attack attack;
//     // std::vector<AbilityBinding> abilities;
// };

// // ## Player

// struct PlayerStats {
//     uint16_t health;
//     uint16_t movementSpeed;
// };

// class Player: public cen::CharacterBody2D {
//     public:
//     static const uint64_t _tid;

//     cen::type_id_t TypeId() const override {
//         return Player::_tid;
//     }

//     uint64_t id;
//     std::string name;
//     Champion champion;
//     PlayerStats stats;

//     Player(
//         uint64_t id,
//         std::string name,
//         Champion champion,
//         PlayerStats stats,
//         Vector2 position
//     );
// };

#endif // CSP_ENTITY_H_