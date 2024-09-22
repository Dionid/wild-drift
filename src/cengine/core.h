#ifndef CENGINE_CORE_H
#define CENGINE_CORE_H

#include <memory>
#include <mutex>
#include <map>
#include <raylib.h>
#include <raymath.h>

namespace cen {

// # Id Types
typedef uint64_t node_id_t;
typedef uint64_t player_id_t;

// # Math
constexpr Vector2 Vector2Up {0, -1};
constexpr Vector2 Vector2Down {0, 1};
constexpr Vector2 Vector2Left {-1, 0};
constexpr Vector2 Vector2Right {1, 0};

inline Vector2 Vector2Abs(Vector2 v) {
    return { fabs(v.x), fabs(v.y) };
};

struct Size {
    float width;
    float height;
};

// # World

struct ScreenResolution {
    int width;
    int height;
};

// # Custom RTTI
typedef uint64_t type_id_t;

class TypeIdGenerator {
public:
    TypeIdGenerator(const TypeIdGenerator&) = delete;
    TypeIdGenerator& operator=(const TypeIdGenerator&) = delete;

    static TypeIdGenerator& getInstance() {
        static TypeIdGenerator instance;
        return instance;
    }

    // Method to get the next ID
    uint64_t getNextId() {
        std::lock_guard<std::mutex> lock(mutex_);
        return ++counter_;
    }

    uint64_t typeZero() {
        return 0;
    }

private:
    TypeIdGenerator() : counter_(0) {}

    type_id_t counter_;
    std::mutex mutex_;
};

class WithType {
    public:
        static const type_id_t _tid;
        virtual type_id_t TypeId() const = 0;
};

// # For future
template<typename T>
struct TypeTag;

template<typename T>
static int getTypeId(T* value) {
    return TypeTag<T>::id;
}

// # Input
struct PlayerInput {
    bool up;
    bool down;
    bool left;
    bool right;

    bool compare(const PlayerInput& other) const {
        return this->up == other.up &&
            this->down == other.down &&
            this->left == other.left &&
            this->right == other.right;
    }

    std::vector<uint8_t> Serialize() const {
        std::vector<uint8_t> buffer;

        buffer.push_back(this->up);
        buffer.push_back(this->down);
        buffer.push_back(this->left);
        buffer.push_back(this->right);

        return buffer;
    }

    void Serialize(std::vector<uint8_t>& buffer) const {
        buffer.push_back(this->up);
        buffer.push_back(this->down);
        buffer.push_back(this->left);
        buffer.push_back(this->right);
    }
};

class PlayerInputManager {
    public:
        player_id_t localPlayerId;
        PlayerInput localPlayerInput;
        std::map<player_id_t, PlayerInput> playerInputs;
};

// # Listeners

template <typename T>
struct Listener {
    std::function<void(T)> trigger;
    int id;

    Listener(
        std::function<void(T)> trigger,
        int id = 0
    ): trigger(trigger), id(id) {}
};

template <typename T>
struct ScopedListener {
    std::function<void(int)> off;
    int listenerId;

    ScopedListener(
        std::function<int(std::unique_ptr<T>)> on,
        std::function<void(int)> off,
        std::unique_ptr<T> listener
    ) {
        this->listenerId = on(
            std::move(listener)
        );
    }

    ~ScopedListener() {
        this->off(listenerId);
    }
};

}

#endif // CENGINE_CORE_H