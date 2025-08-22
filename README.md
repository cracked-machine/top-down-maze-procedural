
Note, if you are using Samba to access the .exe, you may get errors after subsequent builds, i.e. `the parameter is incorrect`.

This is a samba error. The dirty workaround is to restart the samba server after each build

```
sudo systemctl restart smb
```

# Debug

1. install mingw and gdb on windows via msys
2. change to the samba mapped directory - `cd X:\\path\\to\\build\\bin\\`
3. Run gdb with the exe - ` gdb X:\\cpp\\games\\temp\\build\\bin\\ProceduralMaze.exe`
4. Type `run` to start the game.

# Collision Detection

To avoid the dreaded "sticky corners" problem, two seperate bounding boxes are used in a cross formation. A horrizontal box that is slightly wider than the player sprite for the x-axis collision detection, and a vertical box that is slightly higher than the player sprite for the y-axis collision detection:

![](cross_bounding_box.svg)

You must ensure that player movement delta is equal or larger than the corner overlaps of the bounding boxes, otherwise the sticky problem may still occur.

# Finding the nearest neighbours

We need to find the neighbour of a given obstacle block. This is useful for our Cellular Autonomy algorithm but is also useful when we want to destroy neighbouring blocks (placing a bomb or using a pickaxe, for example)


We could search every entity that has an obstacle and then search every other entity that owns an obstacle, and see if they are near. However this would be `O(n2)` complexity and would scale very poorly. A better option would be to use the order of creation and the index to find its left neighbour (N-1), right neighbour, top neighbour (N + y), etc.. 

![alt text](ANS.svg)

Unfortunately, the `entt` registry is not a contiguous container so we cannot simply iterate through the registry. Instead we can pushback the entity id into a vector after each one is created. 

We can then use that contiguous vector to represent the grid entities in a single dimension. The resulting algorithm iterates through the grid entities and for each grid entity, it performs a constant amount of work (checking exactly 8 potential neighbors). 
This worked well. The complexity is O(1) and so is incredably fast and scaled well. Typcially a grid of ~5000 entities complete in just a few milliseconds.

A note about storing the entity id outside of the registry: One should not use `entt::entt_traits<entt::entity>::to_entity(entity)` to get the raw integral value of an entity. This is an internal helper and is not guaranteed to be stable or portable.
The correct and safe way is `entt::entt_traits<entt::entity>::to_integral(entity)`, which always returns the underlying uint32_t (or whatever type is used).

When the registry is cleared, entity values may be recycled or invalidated, so using to_entity can yield undefined or corrupted results, especially if it’s not meant for external use.
Always use `to_integral` for serialization, logging, or storing entity IDs outside the registry. This will prevent corruption and ensure your entity IDs remain valid across restarts and registry clears.

# TODO
## Graphics
- [x] add floor tiles
- [x] add level borders
- [x] add texture for wall tiles
- [ ] experiment with shaders
- [ ] add explosion animation
- [ ] add health bar
## Sound
- [ ] Add walking FX
- [ ] add explosion sound

## Gameplay
- [x] Add bombs
    - [x] Add new Component: Neighbours. Contains 8 uint32 slots for entity ids of neighbouring blocks 
    - [x] Update CA to use the new component
    - [x] the occupied block becomes "explosive"
    - [x] activated when key is pressed/released,
    - [x] Add new Sprite: Explosive that can be placed when block is armed
    - [x] When the timer expires, the neighbours are identified
    - [x] disable the identified neighbours in reaction to the explosion

- [ ] extra bomb pickups
- [ ] add rising water
    - Add blue texture with display dimensions and 50% alpha 
    - Add collision detection with player
    - Set initial position to {0, DISPLAY.y}
    - Subtract Y position at intervals
    - End game when texture collides with player
- [x] add minimap
- [x] finish game area (add proper finish area)
- [ ] win game when player reaches goal (right side of map)
- [ ] support other resolutions (only 1920 x1024 atm)
- [ ] tweak proc gen of level (too many gaps, add difficulty setting)
## Physics
- [x] add player movement velocity
- [x] add player movement acceleration
- [x] fix collision detection to work with rectangle outlines = 0

## General
- [x] Add game states: menu, playing, paused, dead, etc..
- [x] Add death i.e. allow game to end
- [x] add quit/restart game control
- [x] God mode (toggle collsision)
- [x] refactor random system class
- [x] warp player back to spawn if they leave the play area
- [ ] Tidy up settings
- [ ] enable settings.toml 
- [ ] use separate thread for render system class?