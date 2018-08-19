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

As well, if you are running on Ubuntu or a Ubuntu derivative, run the bootstrap.sh script in libuavcan.
If not I guess just figure out the dependancies from there.

This *should* set it up. I do need to verify this on a new install.

## Creating a New Project ##

Always make a new branch with the naming scheme "firstname-project".
If you want to branch from there, use "firstname-project-subproject"
If there are two people with the same first name, use your firstname, then your last name.
So If I (David Lenfesty) wanted to create a project called status_indicator, and there was another david, I would create the branch:
"davidL-status_indicator".

Don't do this without David there, this is just being written down so it isn't forgotten.
You should just be able to cp the libuavcan_example into a new folder and change some names and it should work.
