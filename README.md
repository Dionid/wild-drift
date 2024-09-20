# Raylib Pong

# Roadmap

1. Multiplayer
    1. Thread results
    1. On disconnect
1. Managers as Actors?
1. Signals to send stopping messages down the tree
1. Event bus Thread safety
1. Errors
1. Escape not working in waiting while loop
1. Logger
1. EventBus is now flushing only while scene is working
1. Move SPC logic from .h to .cpp
1. New features
    1. Capsule Collider
    1. AssetsLoader threads
    1. Debug as flag
    1. Settings
    1. Animations
    1. Game Speed
    1. Sprites
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
1. Multiplayer
    1. [Networking Physics (+ description links)](https://www.youtube.com/watch?v=9OjIDko1uzc)
    1. https://gafferongames.com/categories/networked-physics/
    1. https://archive.org/details/GDC2015Fiedler
    1. [8 Frames in 16ms: Rollback Networking in Mortal Kombat and Injustice 2](https://www.youtube.com/watch?v=7jb0FOcImdg)
    1. [Fast-Paced Multiplayer](https://www.gabrielgambetta.com/client-server-game-architecture.html)
    1. ...

# Caution

1. Use global position to know position of the Node in the world (i.e. combining all parent positions).
1. Put Collider directly into ColliderBody2D.
1. Initial nested Nodes must be added in Init method.
1. ...

# Multiplayer

## Variants

1. Lock Step
    1. Clients sends inputs to each other, waits till all inputs are received and simulates
    1. Cons: everybody waits for everybody
1. Input Rollback
    1. Clients sends inputs to each other, if input is invalid, client rolls back to last valid state and resimulates
1. State Sync
    1. Client sends input, server sends state to everybody, client don't simulate
1. (CA-AC) Client Authoritative + Anti-Cheat
    1. Clients simulates everything, server validates
1. PRI
    1. Prediction, Reconciliation, Interpolation both on client and server

## LockStep


1. Only when I clicked on "Host" / "Join" I must create transport

1. Show waiting screen
1. Wait till players joined match
1. Get player id from host
1. Agree on starting tick
1. Countdown
1. Start match
    1. Create Players nodes and assign correct playerIds


----

1. Move FrameStart