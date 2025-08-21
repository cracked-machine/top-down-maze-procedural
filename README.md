
Note, if you are using Samba to access the .exe, you may get errors after subsequent builds, i.e. `the parameter is incorrect`.

This is a samba error. The dirty workaround is to restart the samba server after each build

```
sudo systemctl restart smb
```

# Collision Detection

To avoid the dreaded "sticky corners" problem, two seperate bounding boxes are used in a cross formation. A horrizontal box that is slightly wider than the player sprite for the x-axis collision detection, and a vertical box that is slightly higher than the player sprite for the y-axis collision detection:

![](cross_bounding_box.svg)

You must ensure that player movement delta is equal or larger than the corner overlaps of the bounding boxes, otherwise the sticky problem may still occur.


# TODO
## Graphics
- [x] add floor tiles
- [x] add level borders
- [x] add texture for wall tiles
- [ ] experiment with shaders
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
## Physics
- [x] add player movement velocity
- [x] add player movement acceleration
- [x] fix collision detection to work with rectangle outlines = 0

## General
- [ ] Add "idle" state. i.e. game not running, blank screen with "press any key to start" message"
- [ ] Add death i.e. allow game to end
- [ ] add quick restart (r key)
- [x] God mode (toggle collsision)
- [x] refactor random system class
- [x] warp player back to spawn if they leave the play area
- [ ] Tidy up settings
- [ ] enable settings.toml 