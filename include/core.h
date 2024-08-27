#include <memory>
#include <raymath.h>

#ifndef CENGINE_CORE_H
#define CENGINE_CORE_H

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
};

// # Constants

constexpr Vector2 Vector2Up {0, -1};
constexpr Vector2 Vector2Down {0, 1};
constexpr Vector2 Vector2Left {-1, 0};
constexpr Vector2 Vector2Right {1, 0};

// # Structs

struct Size {
    float width;
    float height;
};

#endif // CENGINE_CORE_H