# ----------------------------
# Makefile Options
# ----------------------------

NAME = bmpb
INIT_LOC = 0B0000
DESCRIPTION = "Buffer-backed .BMP parser and uploader"
COMPRESSED = NO
LDHAS_EXIT_HANDLER:=0

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
