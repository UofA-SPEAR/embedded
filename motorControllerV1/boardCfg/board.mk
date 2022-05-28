# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/SPEAR_SPEEDY_F3/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/os/hal/boards/SPEAR_SPEEDY_F3

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
