#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# Add BUFF_SIZE define for all files
CFLAGS += -DBUFF_SIZE=128