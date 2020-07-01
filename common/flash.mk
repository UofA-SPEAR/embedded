# Adds a `flash` target, for the F3

ifdef OPENOCD_SEARCH_DIR
	OPENOCD_ARGS := -s ${OPENOCD_SEARCH_DIR}
endif

flash: ./build/ch.elf
	openocd -f ../common/f3_openocd.cfg -c "program ./build/ch.elf verify reset exit" ${OPENOCD_ARGS}

# Hack so we don't use this as default

.DEFAULT_GOAL :=
