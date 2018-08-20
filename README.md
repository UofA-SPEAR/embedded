# embedded #

This is all the code for the embedded systems on the rover.

I want to try using STM32 chips on our boards and use STM32CubeMX for configuring them, instead of Arduino, so that I can have some semblence of control over the build process.
Obviously depends on what members we get and how willing they are.

## Running ##

Install SW4STM32 (will add link later) and open up the project. Theoretically everything should be installed.

## Creating a New Project ##

Make sure you have:
- SW4STM32
- STM32CubeMX
- Git and stuff

First, use STM32CubeMX to generate the settings you want for your project (select the chip, enable the required peripherals, etc.

Once you've generated your project, add symlinks to libuavcan and dsdlc_generated from your Inc folder.
Then right-click on the project in your project explorer and click "Convert to C++".

Then go to Project->Properties->C/C++ Build->Settings->Includes and add in your libuavcan, libuavcan_stm32, and dsdlc_generated includes.

After that, head to the preprocessor tab and to the Defined Symbols (-D) tab, add UAVCAN_STM32_FREERTOS=1, UAVCAN_STM32_NUM_IFACES=1, and UAVCAN_STM32_TIMER_NUMBER=1.
(These can be changed but you have to know what you're doing).

Then, head to Project->Properties->Resources->Resource Filters and add four filters. These filters should be exclude all folders and their children, with the filters "linux", "posix", "kinetis", "lpc11c24", "tools", and "test".

Make your chip.h file. More on this later.

I *think* this is it.
