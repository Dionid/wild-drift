# Raylib Pong

# Roadmap

1. Multiplayer
1. Remove raygui
1. New features
    1. Debug as flag
    1. Settings
    1. Animations
    1. AssetsLoader threads
    1. Game Speed
    1. Dialogs
1. Improvements
    1. ? pThreads
    1. Input Buffer
    1. Move render and query pipelines to Scene / World
    1. zOrder on Tree
    1. Node2D position -> transform
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
    1. Change Vector2Add to mutable where possible (like GlobalPosition)
1. Maybe
    1. ? Arena allocator

# Features

1. CharacterBody2D (move and slide, move and collide)
1. Collisions
1. Custom RTTI

# Useful Links

1. [VSCode CLang](https://code.visualstudio.com/docs/cpp/config-clang-mac)
1. [Unity Execution Order](https://docs.unity3d.com/Manual/ExecutionOrder.html)

# Caution

1. Use global position to know position of the Node in the world (i.e. combining all parent positions).
1. Put Collider directly into ColliderBody2D.
1. Initial nested Nodes must be added in Init method.
1. ...