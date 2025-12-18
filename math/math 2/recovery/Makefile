# Recovery System - Master Makefile
# Builds all libraries and tools

CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -I./include
LDFLAGS = -L./lib
LIBS = -lm -lssl -lcrypto -ljansson

# Directories
LIB_DIR = lib
SRC_DIR = src
TOOLS_DIR = tools
INCLUDE_DIR = include
BUILD_DIR = build

# Libraries
LIBS_TO_BUILD = \
	$(LIB_DIR)/recovery_common/librecovery_common.a \
	$(LIB_DIR)/recovery_core/librecovery_core.a \
	$(LIB_DIR)/recovery_crypto/librecovery_crypto.a \
	$(LIB_DIR)/recovery_signal/librecovery_signal.a \
	$(LIB_DIR)/recovery_network/librecovery_network.a

# Tools
TOOLS = \
	$(TOOLS_DIR)/signal-recovery \
	$(TOOLS_DIR)/crypto-recovery \
	$(TOOLS_DIR)/bitcoin-recovery

.PHONY: all clean libs tools test install

all: libs tools

# Build all libraries
libs: $(LIBS_TO_BUILD)

# Build recovery_common library
$(LIB_DIR)/recovery_common/librecovery_common.a:
	@echo "Building recovery_common library..."
	@cd $(LIB_DIR)/recovery_common && $(MAKE)

# Build recovery_core library
$(LIB_DIR)/recovery_core/librecovery_core.a:
	@echo "Building recovery_core library..."
	@cd $(LIB_DIR)/recovery_core && $(MAKE)

# Build recovery_crypto library
$(LIB_DIR)/recovery_crypto/librecovery_crypto.a:
	@echo "Building recovery_crypto library..."
	@cd $(LIB_DIR)/recovery_crypto && $(MAKE)

# Build recovery_signal library
$(LIB_DIR)/recovery_signal/librecovery_signal.a:
	@echo "Building recovery_signal library..."
	@cd $(LIB_DIR)/recovery_signal && $(MAKE)

# Build recovery_network library
$(LIB_DIR)/recovery_network/librecovery_network.a:
	@echo "Building recovery_network library..."
	@cd $(LIB_DIR)/recovery_network && $(MAKE)

# Build tools
tools: libs
	@echo "Building tools..."
	@cd $(TOOLS_DIR) && $(MAKE)

# Run tests
test: all
	@echo "Running tests..."
	@cd tests && $(MAKE) test

# Install to system
install: all
	@echo "Installing recovery system..."
	@mkdir -p /usr/local/lib/recovery
	@mkdir -p /usr/local/include/recovery
	@mkdir -p /usr/local/bin
	@cp -r $(LIB_DIR)/*.a /usr/local/lib/recovery/
	@cp -r $(INCLUDE_DIR)/* /usr/local/include/recovery/
	@cp $(TOOLS_DIR)/signal-recovery /usr/local/bin/
	@cp $(TOOLS_DIR)/crypto-recovery /usr/local/bin/
	@cp $(TOOLS_DIR)/bitcoin-recovery /usr/local/bin/
	@echo "Installation complete!"

# Clean all
clean:
	@echo "Cleaning recovery system..."
	@cd $(LIB_DIR)/recovery_common && $(MAKE) clean 2>/dev/null || true
	@cd $(LIB_DIR)/recovery_core && $(MAKE) clean 2>/dev/null || true
	@cd $(LIB_DIR)/recovery_crypto && $(MAKE) clean 2>/dev/null || true
	@cd $(LIB_DIR)/recovery_signal && $(MAKE) clean 2>/dev/null || true
	@cd $(LIB_DIR)/recovery_network && $(MAKE) clean 2>/dev/null || true
	@cd $(TOOLS_DIR) && $(MAKE) clean 2>/dev/null || true
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# Help
help:
	@echo "Recovery System Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build all libraries and tools (default)"
	@echo "  libs     - Build all libraries"
	@echo "  tools    - Build all tools"
	@echo "  test     - Run test suite"
	@echo "  install  - Install to system (/usr/local)"
	@echo "  clean    - Clean all build artifacts"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Libraries:"
	@echo "  - recovery_common  - Common utilities and loaders"
	@echo "  - recovery_core    - Core recovery algorithms"
	@echo "  - recovery_crypto  - Crypto/Bitcoin recovery"
	@echo "  - recovery_signal  - Signal/I/Q recovery"
	@echo "  - recovery_network - Network recovery"
	@echo ""
	@echo "Tools:"
	@echo "  - signal-recovery  - Signal recovery CLI"
	@echo "  - crypto-recovery  - Crypto recovery CLI"
	@echo "  - bitcoin-recovery - Bitcoin key recovery CLI"