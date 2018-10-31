# embedded #

This is all the code for the embedded systems on the rover.

This code will run on STM32 chips, specifically the F103 or F303 series (preferably F303)

## Running ##

Install [SW4STM32](http://openstm32.org) and import the project.
Theoretically everything should be installed and ready to go.

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
cd embedded
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

### Libcanard ###

Add symlinks to libcanard and dsdlc_generated from your Inc folder.
```
cd project_folder
ln -s ../common/libcanard Inc/libcanard
ln -s ../common/dsdlc_generated/libcanard_dsdlc_generated Inc/dsdlc_generated
```

Then go to Project->Properties->C/C++ Build->Settings->Includes and add in your libcanard, libcanard_stm32, and dsdlc_generated includes.
Relevant folders to include:
- Inc/libcanard/
- Inc/libcanard/drivers/stm32
- Inc/dsdlc_generated

Then, head to Project->Properties->Resources->Resource Filters and add six filters.
These are required so we don't compile unnecessary things at build time that we don't have libraries for.
These filters should be exclude all folders and their children, with the filters:
- nuttx
- avr
- tests
- socketcan

