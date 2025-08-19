
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
- [ ] Add weight to tile pool to affect probability
- [x] add floor tiles
- [x] add level borders
- [x] add texture for wall tiles
## Gameplay
- [ ] Add bombs
    - Add new Component: Neighbours. Contains 8 uint32 slots for entity ids of neighbouring blocks 
    - Update CA to use the new component
    - Add new Component: Explosive. Has count down timer
    - When key is pressed/released, the occupied block becomes "explosive"
    - When the timer expires, the neighbours are all disabled and not rendered. i.e destroyed.
- [ ] add rising water
    - Add blue texture with display dimensions and 50% alpha 
    - Add collision detection with player
    - Set initial position to {0, DISPLAY.y}
    - Subtract Y position at intervals
    - End game when texture collides with player
- [x] add minimap
## Physics
- [ ] add player movement velocity
- [x] fix collision detection to work with rectangle outlines = 0
## General
- [ ] Add "idle" state. i.e. game not running, blank screen with "press any key to start" message"
- [ ] Add death i.e. allow game to end
- [ ] add quick restart (r key)
- [x] God mode (toggle collsision)
- [x] refactor random system class
