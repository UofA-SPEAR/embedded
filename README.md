# embedded #

This is all the code for the embedded systems on the rover.

This code will run on STM32 chips, specifically the F103 or F303 series (preferably F303)

## Dependencies ##

You need to have the following tools installed:

- gcc-arm-none-eabi (GCC arm toolchain)
- make tools
- OpenOCD

## Getting started ##

First we need to clone this repository, and then we can update the submodules within it,
so that we can write the code we need.

```
git clone https://github.com/UofA-SPEAR/embedded
cd embedded
git submodule update --init
```

## Creating a project ##

Copy the example project, embedded/example_project as your own project folder.

Edit the Makefile within your new project to match your project's requirements.

Run:

```
make -j4
```

Congrats! You've built a project!

`make help` will give you more information on all the available targets.

## Debugging/Flashing ##

To flash the firmware, simply run

```
make flash
```

with your programmer and device plugged in.

To debug, simply run:

```
make debug
```

This will launch an OpenOCD instance, with a GDB server running on port 3333.
To actually debug, you will have to attach a GDB instance to it.

### GDB ###

Once we have the OpenOCD server running, we can run GDB with these commands.

```
arm-none-eabi-gdb build/<project_name>.elf
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
```

Now we can debug as we normally would with GDB.

### Eclipse ###

Install Eclipse CDT, and the GNU MCU Eclipse plugin on the Eclipse marketplace. The bare minimum is fine.

Then, create a new debug configuration, with the GDB Hardware Debugging template. Select `build/<project_name>.elf`
as your source, and change the remote target to use localhost port 3333 instead of 10000.

### VSCode ###

You need to install the following VSCode plugins:
- Cortex-Debug
- C/C++

There will be a good config in the .vscode folder within the example project.
Use that for reference.
