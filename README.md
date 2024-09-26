# Raylib Pong

# Roadmap

1. Multiplayer
    1. Thread results
    1. On disconnect
    1. cen::PlayerInputManager to parent class
    1. Loop throw received messages (or index somehow)
    1. Remove coping of network messages
    1. Check Float Precision
    1. (optional) Broadcast from host to clients
    1. P2P
1. Errors
1. uint64_t -> smaller types
1. Move SPC logic from .h to .cpp
1. More private / protected members
1. Managers as Actors
1. Signals to send stopping messages down the tree
1. Event bus Thread safety
1. Logger
1. EventBus is now flushing only while scene is working
1. Debug as flag
1. ? pThreads
1. World
1. Remove raylib
1. (RTTI) Node.id on BitMask
    1. NodeStorage index by class
1. Remove CharacterNode2D.size
1. Release web
1. Name convention (https://google.github.io/styleguide/cppguide.html#General_Naming_Rules)
1. New features
    1. Capsule Collider
    1. AssetsLoader threads
    1. Settings
    1. Animations
    1. Game Speed
    1. Sprites
1. Improvements
    1. Node2D position -> transform
    1. SAT
    1. Add units (meters, seconds, etc.)
    1. CollisionEngine pair.id
    1. NodeGenerator id graveyard
    1. Change Vector2Add to mutable where possible (like GlobalPosition)

# Features

1. CharacterBody2D (move and slide, move and collide)
1. Collisions
1. Custom RTTI
1. LockStep Scene
1. Timers

# Caution

1. Use global position to know position of the Node in the world (i.e. combining all parent positions).
1. Put Collider directly into ColliderBody2D.
1. Initial nested Nodes must be added in Init method.
1. ...

# Useful Links

1. [VSCode CLang](https://code.visualstudio.com/docs/cpp/config-clang-mac)
1. [Unity Execution Order](https://docs.unity3d.com/Manual/ExecutionOrder.html)
1. Multiplayer
    1. [Networking Physics (+ description links)](https://www.youtube.com/watch?v=9OjIDko1uzc)
    1. https://gafferongames.com/categories/networked-physics/
    1. [GDC 2015: Glenn Fiedler - "Physics for Game Programmers : Networking for Physics Programmers"](https://archive.org/details/GDC2015Fiedler)
    1. [8 Frames in 16ms: Rollback Networking in Mortal Kombat and Injustice 2](https://www.youtube.com/watch?v=7jb0FOcImdg)
    1. [Fast-Paced Multiplayer](https://www.gabrielgambetta.com/client-server-game-architecture.html)
    1. ...