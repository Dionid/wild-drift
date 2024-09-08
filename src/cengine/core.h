#ifndef CENGINE_CORE_H
#define CENGINE_CORE_H

#include <memory>
#include <mutex>
#include <raymath.h>

namespace cen {

// # Shared pointers

class MultipleInheritableEnableSharedFromThis: public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis>
{
public:
  virtual ~MultipleInheritableEnableSharedFromThis()
  {}
};

template <typename T>
class enable_shared_from_base
  : public MultipleInheritableEnableSharedFromThis
{
public:
    std::shared_ptr<T> shared_from_this() {
        return std::dynamic_pointer_cast<T>(MultipleInheritableEnableSharedFromThis::shared_from_this());
    }

    template <class Down>
    std::shared_ptr<Down> downcasted_shared_from_this() {
        return std::dynamic_pointer_cast<Down>(MultipleInheritableEnableSharedFromThis::shared_from_this());
    }

    std::weak_ptr<T> weak_from_this() {
        if (auto shared = MultipleInheritableEnableSharedFromThis::shared_from_this()) {
            return std::dynamic_pointer_cast<T>(shared);
        }

        return std::weak_ptr<T>();
    }

    template <class Down>
    std::weak_ptr<Down> downcasted_weak_from_this() {
        return std::dynamic_pointer_cast<Down>(MultipleInheritableEnableSharedFromThis::weak_from_this());
    }
};

// # Constants

constexpr Vector2 Vector2Up {0, -1};
constexpr Vector2 Vector2Down {0, 1};
constexpr Vector2 Vector2Left {-1, 0};
constexpr Vector2 Vector2Right {1, 0};

inline Vector2 Vector2Abs(Vector2 v) {
    return { fabs(v.x), fabs(v.y) };
};

// # Structs

struct Size {
    float width;
    float height;
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

}

#endif // CENGINE_CORE_H