# PHP Recovery Extension Makefile
# This is a helper Makefile - the actual build uses phpize

.PHONY: all build install clean test help

# Default target
all: help

# Build the extension
build:
	@echo "Building PHP Recovery Extension..."
	@if [ ! -f "configure" ]; then \
		echo "Running phpize..."; \
		phpize; \
	fi
	@if [ ! -f "Makefile" ] || [ ! -f "config.status" ]; then \
		echo "Running configure..."; \
		./configure --with-recovery=/usr/local; \
	fi
	@echo "Compiling..."
	@$(MAKE) -f Makefile.phpize

# Install the extension
install: build
	@echo "Installing PHP Recovery Extension..."
	@sudo $(MAKE) -f Makefile.phpize install
	@echo ""
	@echo "Extension installed successfully!"
	@echo "Add 'extension=recovery.so' to your php.ini to enable it."
	@echo ""
	@echo "To verify installation, run:"
	@echo "  php -m | grep recovery"

# Clean build files
clean:
	@echo "Cleaning PHP extension build files..."
	@if [ -f "Makefile.phpize" ]; then \
		$(MAKE) -f Makefile.phpize clean; \
	fi
	@rm -f configure config.* Makefile.phpize
	@rm -rf .libs modules autom4te.cache build
	@rm -f *.lo *.la
	@echo "Clean complete"

# Run tests
test: build
	@echo "Running PHP extension tests..."
	@php -d extension=modules/recovery.so examples/basic_recovery.php
	@echo ""
	@echo "Tests complete"

# Show help
help:
	@echo "PHP Recovery Extension Build System"
	@echo ""
	@echo "Usage:"
	@echo "  make build    - Build the PHP extension"
	@echo "  make install  - Build and install the extension"
	@echo "  make clean    - Clean build files"
	@echo "  make test     - Run example tests"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - PHP development files (php-dev or php-devel)"
	@echo "  - phpize tool"
	@echo "  - Universal Recovery System libraries installed"
	@echo ""
	@echo "Manual build:"
	@echo "  phpize"
	@echo "  ./configure --with-recovery=/usr/local"
	@echo "  make"
	@echo "  sudo make install"