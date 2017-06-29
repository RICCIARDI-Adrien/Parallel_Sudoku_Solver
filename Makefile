PATH_SOURCES = Sources
PATH_INCLUDES = Includes

CC = gcc
CCFLAGS = -W -Wall -Werror

BINARY = Parallel_Sudoku_Solver
INCLUDES = -I$(PATH_INCLUDES)
LIBRARIES = -pthread
SOURCES = $(shell find $(PATH_SOURCES) -iname "*.c")

release: CCFLAGS += -O3 -g
release: all

debug: CCFLAGS += -g -DCONFIGURATION_IS_DEBUG_ENABLED=1
debug: all

all:
	$(CC) $(CCFLAGS) $(INCLUDES) $(SOURCES) -o $(BINARY) $(LIBRARIES)

clean:
	rm -f $(BINARY) $(BINARY).exe
