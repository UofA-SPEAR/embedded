# Adds a `flash` target, for the F3

flash: ./build/ch.elf
	openocd -f ../common/f3_openocd.cfg -c "program ./build/ch.elf verify reset exit"

# Hack so we don't use this as default

.DEFAULT_GOAL :=