# Makefile for Hyper Prime Spiral Application
# Updated to match current flat structure with ui/tabs subdirectory

CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -std=c11 -D_POSIX_C_SOURCE=200809L -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
LDFLAGS = -L.. -L../math/lib -lcrystalline -lcllm -lcrawler -lcrystallinemath -lSDL2 -lSDL2_ttf -lm -lpthread -lsqlite3 -lcurl -fsanitize=address -fsanitize=undefined
INCLUDES = -I../cllm/include/ai -I. -I../math/include -I../algorithms/include

# Source files in flat app/ directory (excluding ui/tabs and test files)
APP_SOURCES = $(filter-out ui/tabs/%.c test_%.c, $(wildcard *.c))
APP_OBJECTS = $(APP_SOURCES:.c=.o)

# Note: text_input.c is included in APP_SOURCES automatically

# Tab source files in ui/tabs/ subdirectory
TAB_SOURCES = $(wildcard ui/tabs/*.c)
TAB_OBJECTS = $(TAB_SOURCES:.c=.o)
   # UI source files in ui/ subdirectory
   UI_SOURCES = $(wildcard ui/*.c)
   UI_OBJECTS = $(UI_SOURCES:.c=.o)
   
   # Crystalline UI source files in ui/crystalline/ subdirectory
   CRYSTALLINE_UI_SOURCES = $(wildcard ui/crystalline/*.c)
   CRYSTALLINE_UI_OBJECTS = $(CRYSTALLINE_UI_SOURCES:.c=.o)

# Crystalline UI source files
CRYSTALLINE_SOURCES = $(wildcard ui/crystalline/*.c)
CRYSTALLINE_OBJECTS = $(CRYSTALLINE_SOURCES:.c=.o)

# All objects
ALL_OBJECTS = $(APP_OBJECTS) $(UI_OBJECTS) $(TAB_OBJECTS) $(CRYSTALLINE_OBJECTS)

# Target executable
TARGET = hyper_prime_spiral

# Default target
all: $(TARGET)

$(TARGET): $(ALL_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(ALL_OBJECTS) -o $(TARGET) $(LDFLAGS) -Wl,-rpath,..,-rpath,.
	@echo "✓ Build complete!"
	@echo ""
	@echo "Run with: LD_LIBRARY_PATH=.. ./$(TARGET)"

# Compile app files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile ui files
ui/%.o: ui/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile tab files
ui/tabs/%.o: ui/tabs/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run the application
run: $(TARGET)
	@echo "Running $(TARGET)..."
	@LD_LIBRARY_PATH=.. ./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Clean
clean:
	@echo "Cleaning application..."
	rm -f $(APP_OBJECTS) $(UI_OBJECTS) $(CRYSTALLINE_UI_OBJECTS) $(TAB_OBJECTS) $(TARGET)
	rm -f ui/*.o ui/tabs/*.o ui/crystalline/*.o
	@echo "✓ Application cleaned"

# Info
info:
	@echo "Application Build Information"
	@echo "=============================="
	@echo "App sources: $(words $(APP_SOURCES)) files"
	@echo "UI sources: $(words $(UI_SOURCES)) files"
	@echo "Tab sources: $(words $(TAB_SOURCES)) files"
	@echo "Total sources: $(words $(APP_SOURCES) $(UI_SOURCES) $(TAB_SOURCES)) files"
	@echo ""
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo ""
	@echo "Targets:"
	@echo "  all   - Build application"
	@echo "  run   - Build and run application"
	@echo "  debug - Build with debug symbols"
	@echo "  clean - Remove build artifacts"
	@echo "  info  - Show this information"

.PHONY: all run debug clean info

# Compile crystalline UI files
ui/crystalline/%.o: ui/crystalline/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
