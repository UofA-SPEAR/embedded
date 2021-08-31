.PHONY: format
format:
	clang-format-12 -i inc/*.h source/*.c
