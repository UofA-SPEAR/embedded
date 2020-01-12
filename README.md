![Travis-CI](https://travis-ci.com/UofA-SPEAR/embedded.svg?branch=master)

# embedded #

This is all the code for the embedded systems on the rover.

This code will run on STM32 chips, specifically the F103 or F303 series (preferably F303)

## Onboarding

1. First, create a GitHub account and send the username to the Controls team lead.
2. Install the [required toolchain](https://github.com/UofA-SPEAR/embedded/wiki/Setting-up-the-Development-Environment)
3. Follow the [Onboarding Task](https://github.com/UofA-SPEAR/embedded/wiki/Onboarding-Task).

## Dependencies ##

You need to have the following tools installed:

- gcc-arm-none-eabi (GCC arm toolchain)
- make tools
- OpenOCD

## Documentation

- [ChibiOS Documentation](http://www.chibios.org/dokuwiki/doku.php?id=chibios:documentation:start) (We are using 19.1.x)
- [STM32F303 Datasheet](https://www.st.com/resource/en/datasheet/stm32f303vc.pdf)
- [STM32F303 Reference Manual](https://www.st.com/content/ccc/resource/technical/document/reference_manual/4a/19/6e/18/9d/92/43/32/DM00043574.pdf/files/DM00043574.pdf/jcr:content/translations/en.DM00043574.pdf) (this is where you go for peripheral information)

### Windows

Install Git Bash

Install the ARM GNU toolchain from the ARM website: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

It works to build the makefile using git Bash, or anything like linux for windows, but I haven't done full integration into windows yet. (mostly because windows development sucks)

## Getting started ##

First we need to clone this repository, and then we can update the submodules within it,
so that we can write the code we need.

```
git clone https://github.com/UofA-SPEAR/embedded
cd embedded
git submodule update --init
```

## Creating a project ##

Copy the example project, `embedded/chibios_example` as your own project folder.

Edit the Makefile within your new project to match your project's requirements.

Run:

```
make -j4
```

Congrats! You've built a project!

`make help` will give you more information on all the available targets.

## Debugging/Flashing ##

### THIS MAY CHANGE SOON ###

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
