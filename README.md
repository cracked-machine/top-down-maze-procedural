# Build System

This project is setup to build on a linux system but cross-compile for a windows target via MinGW. The mingw version is tied to the Debian release. Rather than put out-of-date info here please check the Dockerfile and then look up that package version [here](https://packages.debian.org/search?keywords=g%2B%2B-mingw-w64-x86-64&searchon=names&suite=all&section=all).  

You can determine the CRT by running objdump on the binary output:

```
x86_64-w64-mingw32-objdump -p build-x86_64-w64-mingw32/bin/ProceduralMaze.exe | grep -iE 'msvcrt|ucrt|vcruntime|api-ms-win-crt'
```

The OpenGL loader used entirely depends on the runtime OS. This is printed in the log file when the game is executed. For example on Windows 11:

```
Engine.cpp:57 - OpenGL: 4.6.0 Compatibility Profile Context 26.1.1.251223
Engine.cpp:58 - Renderer: AMD Radeon RX 7900 GRE
Engine.cpp:59 - Vendor: ATI Technologies Inc.
```

This project should be run on a Linux PC. The cross-compiled output binary for Windows can be found in `build-x86_64-w64-mingw32/bin` and the native output binary for Linux is `build-x86_64-linux-gnu/bin`.

The project ius configured to use VSCode remote extensions. This means you can run the build on a Linux PC from a Windows PC over SSH. You can build it natively from Linux if you wish but obviously if you want to test the cross-compiled windows binary you will need a Windows PC as well. Also note that this project is intended to be used from a Windows PC so of you run it directly in VSCode on Linux the task.json are not tested. 



## Samba

This lets you create a share on the Linux build host and run the executable from Windows explorer. Your `/etc/samba/smb.conf` should look like this:

```
[projects]
        path = /home/chris/projects
        browseable = yes
        read only = no
        writeable = yes
        # Disable oplocks entirely for better executable performance
        oplocks = no
        level2 oplocks = no
        kernel oplocks = no
        strict locking = no
        posix locking = no
        map archive = no
        map hidden = no
        map system = no
        # Add these for better performance
        socket options = TCP_NODELAY IPTOS_LOWDELAY SO_RCVBUF=131072 SO_SNDBUF=131072
        min receivefile size = 16384
```

Note we are disabling oplocks to prevent errors - `the parameter is incorrect` - when you rebuild the executable.  (see https://www.samba.org/samba/docs/current/man-html/smb.conf.5.html)


If you still see this error, the last resort is to ssh into the build host and restart the samba server after each build(!)

```
sudo systemctl restart smb
```

## Running the build in a debugger

In theory you can get windows to create a crash dump, but trying to use this windows-only crash dump with MinGW gcc symbols is a non-starter. Much simpler is to run the executable in GDB directly and observe the debugger when it crashes. Obviously this isn't much help after the fact.  

1. install mingw and gdb on windows via msys - the version must be equal or higher than the mingw supplied by debian.
1. Download the bin folder to your Windows PC (or use the Samba mapping)

        - `cd /c/path/to/bin`
        - `cd X:\\path\\to\\build\\bin\\`

1. Run gdb with the exe - 

        - `gdb X:\\cpp\\games\\temp\\build\\bin\\ProceduralMaze.exe -ex run`
        - `/c/path/to/bin/ProceduralMaze.exe -ex run`

1. Or run with a breakpoint - 

        - `gdb /x/cpp/games/top-down-maze-procedural/build/bin/ProceduralMaze.exe -ex "break Engine.cpp:30" -ex "run" -ex "next" -ex "next"`
        - `gdb /c/path/to/bin/ProceduralMaze.exe -ex "break Engine.cpp:30" -ex "run" -ex "next" -ex "next"`

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

## JSON

To add Sprite types, edit the res/json/sprite_metadata_schema.json file
and then update the mapping function (string_to_sprite_type) in src/sprite/sprite_factory.cpp

## Installing and launching the game from remote build server

After building the exe on a remote Linux build server, it is recommended to copy (install) the files onto the local windows workstation before launching the game. For performance reasons, it is not recommended to launch the game over a network/samba share. Using SCP can speed up this copy/install process:

```
del /q "C:\Users\chris\Desktop\Resurrectionist\bin"
mkdir "C:\Users\chris\Desktop\Resurrectionist\bin"
scp -r 192.168.1.106:/home/chris/projects/cpp/games/top-down-maze-procedural\build-x86_64-w64-mingw32\bin C:\Users\chris\Desktop\Resurrectionist

start /wait /b /d "C:\Users\chris\Desktop\Resurrectionist\bin\" ProceduralMaze.exe
```
