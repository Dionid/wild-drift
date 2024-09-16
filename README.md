# Raylib Pong

# Roadmap

1. Multiplayer
    1. Thread results
    1. Move TicksManager somewhere
1. Remove raygui
1. New features
    1. Debug as flag
    1. Settings
    1. Capsule Collider
    1. Animations
    1. AssetsLoader threads
    1. Game Speed
    1. Sprites
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
1. Multiplayer
    1. [Networking Physics (+ description links)](https://www.youtube.com/watch?v=9OjIDko1uzc)
    1. https://gafferongames.com/categories/networked-physics/
    1. https://archive.org/details/GDC2015Fiedler
    1. [Fast-Paced Multiplayer](https://www.gabrielgambetta.com/client-server-game-architecture.html)
    1. ...

# Caution

1. Use global position to know position of the Node in the world (i.e. combining all parent positions).
1. Put Collider directly into ColliderBody2D.
1. Initial nested Nodes must be added in Init method.
1. ...

# Multiplayer

1. General
    1. Clients sending
    1. ! Ticks must be synchronized between Clients and Server
1. Server
    1. ! Imitates second player as AI in Solo
    1. Creates entities
    1. Receives only inputs and RPC-s from Clients
    1. Sends world state to Clients
    1. ! Match reset will be sync points
1. Clients
    1. After receiving world state validate it with what it got
        1. We need to save inputs with tick number (TickInputState)
        1. We can save transform and velocity in the end every tick, with tick number (TickGameState)
        1. ! TickState is also what Server sends us
        1. We compare TickState with what we got from Server
        1. If it is invalid, than we replay every input from last valid TickGameState and set it to current GameState
1. Problems
    1. How to create and destroy entities


## Variants

1. Every client simulates world and sends input to each other throw Server
    1. Cons
        1. Clients can desynchronize pretty quickly
1. Clients sending inputs to Server and Server sends back world state
    1. Cons
        1. Latency
1. Clients simulate world, send inputs to Server and Server simulates world and sends back world state
    1. Cons
        1. Hard to implement



1. Compare pending and arrived GameStateTicks
    1. Get valid pending
    1. Get invalid pending
1. If invalid state exists
    1. Rollback
        1. Revert all pending states till invalid one
        1. Empty all pending states
        1. Apply all arrived GameStateTicks (save lastValidTick)
        1. Simulate using inputs from last valid GameStateTick
1. If not
    1. Remove validated GameStateTicks & InputTicks including validated itself


1. 3 steps: Prediction, Interpolation, Reconciliation
1. Prediction
    1. 2 cases:
        1. Other players
            1. We can make InputManager that will give us last input (locally it would be the same as current input) and apply it to player
        1. Bots
            1. We will just use local AI logic and apply new from server
1. Reconciliation
    1. Process
        1. Check last server GameStateTick
        1. Rollback to that tick canceling new events
        1. Apply last server GameStateTick
        1. Resimulate using inputs from last valid GameStateTick
    1. 2 cases:
        1. Other players
        1. Bots
            1. Just apply from server and resimulate
1. Interpolation
    1. ...



1. Variations
    1. Lock Step
        1. Everybody waits for everybody
    1. State Sync
        1. Server sends state to everybody, client don't simulate
    1. (CA-AC) Client Authoritative + Anti-Cheat
        1. Clients simulates everything, server validates


1. CA-AC
    1. Mechanics
        1. Clients sending
            1. Their positions + events to Server, server propagates them to other clients
    1. Questions
        1. Who will be responsible for simulating grenades?
        1. How mobs are simulated? (maybe they should be simulated on server)


1. My goal
    1. Make multiplayer as smooth as possible and as easy to implement as possible
    1. Move a lot of computations to server (for mobile)


# PRI (Prediction, Reconciliation, Interpolation)

! You interpolate enemy position in interpolation way

1. Get local Input
1. Send Input
1. Get GameStateTick
1. Compare
1. Rollback (canceling all pending states)
1. Resimulate
1. Simulate locally
1. Save GameStateTick



1. Click "Solo"
1. Server send classes _tids;
1. Server spawns player, enemy and ball


!!! Sync ids

1. Если я создал объект на клиенте
    1. Могу ли я просто поверить клиенту и отправить ему мапинг id?
    1. Может надо разделить понятия на расчет физики и остальную игровую логику?

1. !!! Если я буду обмениваться Ивентами по UDP, то они будут просто проебываться
    1. Переотправлять все события до получения одобрения?


Ok, может все-таки Lock Step попробовать?

1. Все клиенты должны получить все input чтобы симулировать шаги

