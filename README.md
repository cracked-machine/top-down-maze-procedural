
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

- [ ] add player movement velocity
- [ ] Add bombs
- [ ] add rising water
- [ ] add quick restart (r key)
- [ ] add minimap
- [ ] add floor tiles
- [x] add level borders
- [ ] refeactor random system class
- [ ] start screen
- [ ] hitbox intersection using rectangle fill size not outline
