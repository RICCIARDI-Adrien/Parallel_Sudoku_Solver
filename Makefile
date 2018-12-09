PATH_SOURCES = Sources
PATH_INCLUDES = Includes

CC = gcc
CCFLAGS = -W -Wall

BINARY = Parallel_Sudoku_Solver
INCLUDES = -I$(PATH_INCLUDES)
LIBRARIES = -pthread
SOURCES = $(shell find $(PATH_SOURCES) -iname "*.c")

release: CCFLAGS += -Werror -O3 -fomit-frame-pointer -finline-functions-called-once -mtune=native
release: all

debug: CCFLAGS += -g -O0 -DCONFIGURATION_IS_DEBUG_ENABLED=1
debug: all

all:
	$(CC) $(CCFLAGS) $(INCLUDES) $(SOURCES) -o $(BINARY) $(LIBRARIES)

clean:
	rm -f $(BINARY) $(BINARY).exe
