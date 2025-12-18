# Makefile for Geometric Threading 88D Prototype

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I../../include -I../../math/include -I../../algorithms/include
LDFLAGS = -L../../math/lib -L../../ -L../../algorithms
LIBS = -lcrystallinemath -lalgorithms -lm -lpthread

# Source files
SOURCES = geometric_ops.c
OBJECTS = $(SOURCES:.c=.o)

# Test files
TEST_SOURCES = test_geometric_ops.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_BINARY = test_geometric_ops

.PHONY: all clean test

all: $(TEST_BINARY)

$(TEST_BINARY): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BINARY)
	LD_LIBRARY_PATH=../../math/lib:../../:../../algorithms ./$(TEST_BINARY)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TEST_BINARY)

# Dependencies
geometric_ops.o: geometric_ops.c geometric_ops.h geometric_types.h
test_geometric_ops.o: test_geometric_ops.c geometric_ops.h geometric_types.h