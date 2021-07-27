![Travis-CI](https://travis-ci.com/UofA-SPEAR/embedded.svg?branch=master)

# embedded #

This is all the code for the embedded systems on the rover.

This code will run on STM32 chips, specifically the F103 or F303 series (preferably F303)

## Onboarding

1. First, create a GitHub account and send the username to the Controls team lead.
2. Install the [required toolchain](https://github.com/UofA-SPEAR/embedded/wiki/Setting-up-the-Development-Environment)
3. Follow the [Onboarding Task](https://github.com/UofA-SPEAR/embedded/wiki/Onboarding-Task).

## Requirements ##

You need to have the following tools installed:

- Oracle Virtualbox
- Oracle Virtualbox Extension Pack
- STlink Driver
- Vagrant
- VScode
- Git
- At least 4GB of storage space

## Documentation

- [ChibiOS Documentation](http://www.chibios.org/dokuwiki/doku.php?id=chibios:documentation:start) (We are using 19.1.x)
- [STM32F303 Datasheet](https://www.st.com/resource/en/datasheet/stm32f303vc.pdf)
- [STM32F303 Reference Manual](https://www.st.com/content/ccc/resource/technical/document/reference_manual/4a/19/6e/18/9d/92/43/32/DM00043574.pdf/files/DM00043574.pdf/jcr:content/translations/en.DM00043574.pdf) (this is where you go for peripheral information)

### Windows

Install listed dependencies

It works by building a virtual machine via Vagrant that contains most of the required toolchain.

## Getting started ##

- First clone this repository to the machine you will work on.

```
git clone https://github.com/UofA-SPEAR/embedded
```

- Navigate to the root directory of the embedded project via cmd terminal/bash.
- Run `vagrant up`. The script will now setup the VM, this process may take a while. Ensure that you have internet access as it will download the VM.
- Run `vagrant ssh-config` to retreive the ssh config for the VM.
- Install the `Remote - SSH` extension for VScode. See the VSCode section below for more info.
- In the command pallet (ctrl + shift + p) type `remote-ssh: open configuration file` and select the first option
- Copy the output of `vagrant ssh-config` to the file that was opened.
- In the command pallet (ctrl + shift + p) type `remote-ssh: connect to host` or click on the green icon in the bottom left corner of the VScode window.
- Once connected, go find the `c/c++` and `cortex-debug` extension in the VScode extension marketplace, and use the option 'install in spear-embedded-box' and reload the window once completed.
- Go to `terminal > run task > build and create c_cpp_properties` in the reloaded window to generate VScode json.


## Creating a project ##

Copy the example project, `embedded/template_project` as your own project folder.

## Debugging/Flashing ##

### UPDATE THIS ###

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

If you're using a pure open-source version of vscode, then you'll need this workaround in order to access the marketplace: https://github.com/VSCodium/vscodium/issues/418#issuecomment-643664182.
Otherwise, you'll get an error like: "We cannot connect to the Extensions Marketplace at this time, please try again later".
You may also need to do this workaround: https://docs.microsoft.com/en-us/visualstudio/liveshare/reference/linux#vs-code-oss-issues if you get errors like "Command ... not found".

If you are using Arch, there is a AUR package to do this patching automatically: https://aur.archlinux.org/packages/code-marketplace/

If you continue to run into issues with the open-source version, you may need to switch to the microsoft version of vscode.

### UPDATE THIS ###
