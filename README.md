# embedded #

This is all the code for the embedded systems on the rover.

## Running ##

Download and install [platformio](https://platformio.org).
You can use whatever text editor you want, recommended ones are atom or vscode.
If you're a little more advanced and want to use vim and platformio on the commandline go right on ahead.

Run these commands to clone the repository and set up the required libraries:

```
git clone https://github.com/UofA-SPEAR/embedded.git
cd embedded/libuavcan
git submodule update --init --recursive
```

This *should* set it up. I do need to verify this on a new install.

## Creating a New Project ##

Don't do this without David there, this is just being written down so it isn't forgotten.
You should just be able to cp the libuavcan_example into a new folder and change some names and it should work.
