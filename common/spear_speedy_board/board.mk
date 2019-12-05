# List of all the board related files.
BOARDSRC = $(COMMON)//spear_speedy_board/board.c

# Required include directories
BOARDINC = $(COMMON)/spear_speedy_board

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
