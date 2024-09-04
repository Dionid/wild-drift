# Raylib Pong

# Roadmap

1. Node::Init
1. Menu
1. Audio
1. Release (macOS, Windows, Linux, web)
1. Add units (meters, seconds, etc.)
1. Remove raylib
1. Fixed update
1. Multithreading
1. Name convention (https://google.github.io/styleguide/cppguide.html#General_Naming_Rules)
1. SAT
1. Z index
1. ? Arena allocator

# Features

1. CharacterBody2D (move and slide, move and collide)
1. Collisions
1. Custom RTTI

# Questions

1. Do I need size in CharacterNode2D?

# Useful Links

1. [VSCode CLang](https://code.visualstudio.com/docs/cpp/config-clang-mac)

# Global Position

1. To achieve position of nested Nodes, we need to calculate their global position.
This global position will be used in collision detection and rendering.