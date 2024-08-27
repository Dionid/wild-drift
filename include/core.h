#include <raymath.h>

#ifndef CENGINE_CORE_H
#define CENGINE_CORE_H

constexpr Vector2 Vector2Up {0, -1};
constexpr Vector2 Vector2Down {0, 1};
constexpr Vector2 Vector2Left {-1, 0};
constexpr Vector2 Vector2Right {1, 0};

struct Size {
    float width;
    float height;
};

#endif // CENGINE_CORE_H