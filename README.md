# Wild-drift

# Roadmap

1. Move camera
1. Player
1. y-sorting
1. ...

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