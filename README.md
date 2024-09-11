# Raylib Pong

# Roadmap

1. New features
    1. Multithreading
        1. Physics and Render pipelines
        1. Move zOrder sort to render
        1. Fixed Update
    1. Multiplayer
    1. Debug as flag
    1. Settings
    1. Animations
1. Improvements
    1. Input Buffer
    1. zOrder on Tree
    1. SAT
    1. Add units (meters, seconds, etc.)
    1. CollisionEngine pair.id
    1. NodeGenerator id graveyard
    1. Remove raylib
    1. Remove CharacterNode2D.size
    1. Defer Node Add & Remove
    1. (RTTI) Node.id on BitMask
        1. NodeStorage index by class
    1. Name convention (https://google.github.io/styleguide/cppguide.html#General_Naming_Rules)
    1. Release (Windows, Linux, web)
1. Maybe
    1. ? Arena allocator

# Features

1. CharacterBody2D (move and slide, move and collide)
1. Collisions
1. Custom RTTI

# Useful Links

1. [VSCode CLang](https://code.visualstudio.com/docs/cpp/config-clang-mac)

# Caution

1. Use global position to know position of the Node in the world (i.e. combining all parent positions).
1. Put Collider directly into ColliderBody2D.
1. Initial nested Nodes must be added in Init method.
1. ...