LIBCANARD := $(COMMON)/libcanard

ALLCSRC += $(LIBCANARD)/canard.c \
           $(LIBCANARD)/drivers/stm32/canard_stm32.c

ALLINC  += $(LIBCANARD) \
           $(LIBCANARD)/drivers/stm32
