# embedded #

This is all the code for the embedded systems on the rover.

I want to try using STM32 chips on our boards and use STM32CubeMX for configuring them, instead of Arduino, so that I can have some semblence of control over the build process.
Obviously depends on what members we get and how willing they are.

## Running ##

Install [SW4STM32](http://openstm32.org) and import the project.
Theoretically everything should be installed and ready to go.

## Importing a project ##

First, run the git command:

```
git submodule update --init
```

Then create symlinks within the project you are using.
Git doesn't preserve symlinks.

```
cd project_name/Inc
ln -s ../../common/libuavcan libuavcan
ln -s ../../common/dsdlc_generated dsdlc_generated
```

Then you should be able to build your project.
If you get build errors like "undefined reference to 'blah blah blah'",
you need to clean your build environment.
So, select Project->Clean..., and click OK, then try and rebuild your project.

## Creating a New Project ##

Before starting any of this, read through it and make sure you understand at least approximately what you're doing.
If you don't, just talk to the electrical lead (David at the moment) and we'll walk through it together.

Before you start, make sure you have:
- SW4STM32
- STM32CubeMX (possibly included with SW4STM32)
- Git

Then, in the folder you want this repo stored in:
```
git clone https://github.com/UofA-SPEAR/embedded.git
cd libuavcan
git submodule update --init --recursive
```

### STM32CubeMX ###

First, use STM32CubeMX to generate the settings you want for your project (select the chip, enable the required peripherals, etc.)

Open up STM32CubeMX and start a new project. Narrow down which chip you will be using (generally stm32f103).
The quickest way to do this is to select your series, then your line, then your package if you know what you want.

Then open your project settings and change your base directory to this repository, and name your project something descriptive, using underscores.
Avoid words like "design" or "board", we know what these are supposed to run on.

After that, you can start configuring your peripherals. Keep it as simple as possible and don't enable a peripheral unless you know you'll need it.
Peripherals can always be added in later it's just a bit more work to do manually (manually does mean you are forced to understand it more though).

If you want FreeRTOS, check the box in the "Middlewares" category.

If you're unsure about different peripherals, consult the datasheet or reference manual from ST.

At this point you can click "Generate Code".
PLEASE LISTEN TO ANY WARNINGS OR ERRORS IT GIVES YOU.
If you don't understand, do more research.

### SW4STM32 Project ###

Click File->Open Projects from File System..., from there navigate to your generated project folder and select that.

Once you've imported the folder, right click on the folder in your Project Explorer view and select "Convert to C++"
Verify that your C++ build settings are sane.

Rename main.c to main.cpp and add extern guards to your includes.
Any pre-included .h files should have these surrounding them:

```
#ifdef __cplusplus
extern "C" {
#endif

// Put your C includes here

#ifdef __cplusplus
}
#endif
```

(Yes those are two underscores)

As well, to reduce code size, check out [this thred.](https://groups.google.com/forum/#!topic/uavcan/18_GhJchWX4)
I will add specific details later once I do more looking into it.

### Libuavcan ###

Add symlinks to libuavcan and dsdlc_generated from your Inc folder.
```
cd project_folder
ln -s ../common/libuavcan Inc/libuavcan
ln -s ../common/dsdlc_generated dsdlc_generated
```

Then go to Project->Properties->C/C++ Build->Settings->Includes and add in your libuavcan, libuavcan_stm32, and dsdlc_generated includes.
Relevant folders to include:
- Inc/libuavcan/libuavcan/include
- Inc/libuavcan/libuavcan_drivers/stm32/driver/include
- Inc/dsdlc_generated

After that, head to the preprocessor tab and to the Defined Symbols (-D) tab, add the necessary defines:
(These can be changed but you have to know what you're doing).

Defaults:
- UAVCAN_STM32_BAREMETAL=1
- UAVCAN_STM32_NUM_IFACES=1
- UAVCAN_STM32_TIMER_NUMBER=1

Then we can head to the Miscellaneous tab and add the following to the "Other Flags" field:

```
-fmessage-length=0 -std=gnu++11 -nodefaultlibs -lc -lgcc -nostartfiles --specs=nano.specs -flto -Os -g3 -ffunction-sections -fdata-sections -fno-common -fno-exceptions -fno-unwind-tables -fno-stack-protector -fomit-frame-pointer -ftracer -ftree-loop-distribute-patterns -frename-registers -freorder-blocks -fconserve-stack -fno-rtti -fno-threadsafe-statics
```

Then, head to Project->Properties->Resources->Resource Filters and add six filters.
These are required so we don't compile unnecessary things at build time that we don't have libraries for.
These filters should be exclude all folders and their children, with the filters:
- linux
- posix
- kinetis
- lpc11c24
- tools
- test

### chip.h ###

Refer to libuavcan_example/Inc/chip.h.
This folder is required by the stm32 libuavcan driver.
Reference [here](https://github.com/UAVCAN/libuavcan_stm32) for more info on this (and the defines further up).

As far as I'm aware this is it, but we'll see.
