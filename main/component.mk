#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# Add BUFF_SIZE define for all files
CFLAGS += -DBUFF_SIZE=128

COMPONENT_SRCDIRS += $(PROJECT_PATH)/components
COMPONENT_ADD_INCLUDEDIRS += $(PROJECT_PATH)/components
# COMPONENT_DIRS := $(PROJECT_PATH)/components $(EXTRA_COMPONENT_DIRS) $(IDF_PATH)/components $(PROJECT_PATH)/main $(IDF_PATH)/LibDev