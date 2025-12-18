# Makefile for Geometric Core

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I. -I../../include -I../../math/include
LDFLAGS = -lm -lpthread

# Core sources
CORE_SOURCES = core/point.c core/angle.c core/scale.c
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)

# Test sources
TEST_SOURCES = tests/test_core.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_BINARY = tests/test_core

.PHONY: all clean test

all: $(TEST_BINARY)

$(TEST_BINARY): $(CORE_OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BINARY)
	./$(TEST_BINARY)

clean:
	rm -f $(CORE_OBJECTS) $(TEST_OBJECTS) $(TEST_BINARY)
	rm -f core/*.o operations/*.o structures/*.o space/*.o compute/*.o models/*.o tests/*.o

# Dependencies
core/point.o: core/point.c core/point.h
core/angle.o: core/angle.c core/angle.h core/point.h
core/scale.o: core/scale.c core/scale.h core/point.h
tests/test_core.o: tests/test_core.c core/point.h core/angle.h core/scale.h