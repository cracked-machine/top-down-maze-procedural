# Build System

This project is setup to build on a linux system but cross-compile for a windows target via MinGW.

You can use the VSCode project with the remote containers extension to build the project on a remote linux build host from a local windows client.  Theres nothing to stop you using Linux for both host and client, but obviously you need somewhere to run the windows executable.

## Samba

This lets you create a share on the Linux build host and run the executable from Windows explorer. Your `/etc/samba/smb.conf` should look like this:

```
[projects]
        path = /home/chris/projects
        browseable = yes
        read only = no
        writeable = yes
        oplocks = no
        level2 oplocks = no
        kernel oplocks = no
        strict locking = no
        posix locking = no
```

Note we are disabling oplocks to prevent errors - `the parameter is incorrect` - when you rebuild the executable.  (see https://www.samba.org/samba/docs/current/man-html/smb.conf.5.html)


If you still see this error, the last resort is to ssh into the build host and restart the samba server after each build(!)

```
sudo systemctl restart smb
```

## Running the build in a debugger

In theory you can get windows to create a crash dump, but trying to use this windows-only crash dump with MinGW gcc symbols is a PITA. Much simpler is to run the executable in GDB directly and observe the debugger when it crashes. Obviously this isn't much help after the fact.  

1. install mingw and gdb on windows via msys
2. change to the samba mapped directory - `cd X:\\path\\to\\build\\bin\\`
3. Run gdb with the exe - ` gdb X:\\cpp\\games\\temp\\build\\bin\\ProceduralMaze.exe`
4. Type `run` to start the game.

# Design 

## Finding the nearest neighbours

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
- [ ] Add audio FX
    - [ ] walking on dirt
    - [ ] walking underwater
    - [ ] lit fuse
    - [ ] bomb detonation
    
## Gameplay
- [x] Add bombs
    - [x] Add new Component: Neighbours. Contains 8 uint32 slots for entity ids of neighbouring blocks 
    - [x] Update CA to use the new component
    - [x] the occupied block becomes "explosive"
    - [x] activated when key is pressed/released,
    - [x] Add new Sprite: Explosive that can be placed when block is armed
    - [x] When the timer expires, the neighbours are identified
    - [x] disable the identified neighbours in reaction to the explosion

- [ ] limit number of bombs and add extra bomb pickups to the game area
- [x] add rising water
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
- [ ] fix movement jank
    - [ ] release
    - [ ] relwithdbginfo
    - [ ] debug
- [ ] player floats when underwater

## General
- [x] Add game states: menu, playing, paused, dead, etc..
- [x] Add death i.e. allow game to end
- [x] add quit/restart game control
- [x] God mode (toggle collsision)
- [x] refactor random system class
- [x] warp player back to spawn if they leave the play area
- [ ] Tidy up settings, move to components
- [ ] enable settings.toml 
- [ ] use separate thread for render system class?