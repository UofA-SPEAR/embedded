DSDL_GENERATED := $(COMMON)/uavcan_dsdl/libcanard_dsdlc_generated

# We can do a find here because all the names are namespaced automatically
ALLCSRC += $(shell find $(DSDL_GENERATED) -name "*.c")
ALLINC  += $(DSDL_GENERATED)
