# Makefile for Platonic Solids Demonstration

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../math/include -I../../include
LDFLAGS = -L../../math/lib -lcrystallinemath -lm

TARGET = platonic_demo
SOURCE = platonic_demo.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

run: $(TARGET)
	LD_LIBRARY_PATH=../../math/lib:$$LD_LIBRARY_PATH ./$(TARGET)

clean:
	rm -f $(TARGET)

help:
	@echo "Platonic Solids Demonstration"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the demo"
	@echo "  make run      - Build and run the demo"
	@echo "  make clean    - Remove built files"
	@echo "  make help     - Show this help message"