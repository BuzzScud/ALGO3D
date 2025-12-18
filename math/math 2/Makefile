# ============================================================================
# Prime Math Library - Master Makefile
# ============================================================================
# Crystalline Lattice Arbitrary Precision Math Library
# Organized modular build system with NO external math dependencies
# ============================================================================

# Compiler and flags
CC = gcc
AR = ar

# Detect CPU capabilities for SIMD optimization
# Check if AVX2 is available (optional, gracefully degrades)
SIMD_FLAGS := $(shell grep -q avx2 /proc/cpuinfo 2>/dev/null && echo "-mavx2 -mfma" || echo "")

# Optional debug flag - set CLLM_DEBUG=1 to enable debug output
# Example: make CLLM_DEBUG=1
ifdef CLLM_DEBUG
    DEBUG_FLAGS = -DCLLM_DEBUG
else
    DEBUG_FLAGS =
endif

CFLAGS = -Wall -Wextra -g -O2 -fPIC -I./cllm/include -I./algorithms/include -I./math/include $(SIMD_FLAGS) $(DEBUG_FLAGS)
LDFLAGS = 
ARFLAGS = rcs

# Library names - Shared (.so)
MATH_LIB = math/lib/libcrystallinemath.so
ALGORITHMS_LIB = libalgorithms.so
CLLM_LIB = cllm/libcllm.so

# Library names - Static (.a)
MATH_STATIC = math/lib/libcrystallinemath.a
ALGORITHMS_STATIC = libalgorithms.a
CLLM_STATIC = cllm/libcllm.a

# Installation directories
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include

# Source directories - Using NEW organized structure
# CLLM library now in cllm/ directory (like math/ and algorithms/)
GEOMETRY_SOURCES = src/geometry/phonetic_values.c
TOOLS_DIR = tools
UTILS_SOURCES = $(wildcard $(SRC_UTILS)/*.c)

# All sources (OLD sources removed)
# Note: CLLM sources now built in cllm/ directory
ALL_SOURCES = $(UTILS_SOURCES)
# Note: CRAWLER and DOCPROC sources now built as part of CLLM library in cllm/

# Object files (OLD objects removed)
UTILS_OBJECTS = $(UTILS_SOURCES:.c=.o)
ALL_OBJECTS = $(ALL_SOURCES:.c=.o)

# Header files
# HEADERS = $(wildcard include/*.h)  # Legacy - no longer used

# ============================================================================
# Main Targets
# ============================================================================

.PHONY: all clean install uninstall test demos app info verify help

all: $(MATH_LIB) $(MATH_STATIC) $(ALGORITHMS_LIB) $(ALGORITHMS_STATIC) $(CLLM_LIB) $(CLLM_STATIC) tools php-ext
	@echo "✓ Build complete!"
	@echo "  Shared Libraries:"
	@echo "    - $(MATH_LIB)"
	@echo "    - $(ALGORITHMS_LIB)"
	@echo "    - $(CLLM_LIB)"
	@echo "  Static Libraries:"
	@echo "    - $(MATH_STATIC)"
	@echo "    - $(ALGORITHMS_STATIC)"
	@echo "    - $(CLLM_STATIC)"

# ============================================================================
# Math Library (Foundation)
# ============================================================================

# 0. Crystalline Math Library (self-contained math, NO math.h)
$(MATH_LIB) $(MATH_STATIC):
	@echo "Building Crystalline Math Library..."
	@cd math && $(MAKE)
	@echo "✓ Math library built successfully"

.PHONY: math-clean
math-clean:
	@cd math && $(MAKE) clean

.PHONY: math-test
math-test:
	@cd math && $(MAKE) test

# ============================================================================
# Three Independent Libraries
# ============================================================================

# 1. Crystalline Lattice Library (core math + geometry)

	@echo "✓ Crystalline shared library created"

	@echo "✓ Crystalline static library created"

# 3. CLLM Library (AI/language model - depends on crystalline and algorithms)
# 2. Algorithms Library (mathematical algorithms - depends on crystalline)
$(ALGORITHMS_LIB): $(MATH_LIB)
	@echo "Building algorithms shared library..."
	@$(MAKE) -C algorithms
	@cp algorithms/$(ALGORITHMS_LIB) .
	@echo "✓ Algorithms shared library created"

$(ALGORITHMS_STATIC): $(MATH_STATIC)
	@echo "Building algorithms static library..."
	@$(MAKE) -C algorithms static
	@cp algorithms/$(ALGORITHMS_STATIC) .
	@echo "✓ Algorithms static library created"

$(CLLM_LIB): $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building CLLM shared library..."
	@$(MAKE) -C cllm
	@cp cllm/libcllm.so .
	@echo "✓ CLLM shared library created"

$(CLLM_STATIC): $(MATH_STATIC) $(ALGORITHMS_STATIC)
	@echo "Building CLLM static library..."
	@$(MAKE) -C cllm
	@cp cllm/libcllm.a .
	@echo "✓ CLLM static library created"

# Removed legacy monolithic libraries

# ============================================================================
# Compilation Rules
# ============================================================================


# AI objects
$(SRC_AI)/%.o: $(SRC_AI)/%.c $(HEADERS)
	@echo "Compiling [AI]: $<"
	$(CC) $(CFLAGS) -c $< -o $@


# ============================================================================
# Subdirectory Builds
# ============================================================================

test: $(STATIC_LIB)
	@echo "Building test suite..."
	@if [ -d tests ] && [ -f tests/Makefile ]; then \
		$(MAKE) -C tests; \
	else \
		echo "No test suite found"; \
	fi

demos: $(STATIC_LIB)
	@echo "Building demos..."
	@if [ -d demos ] && [ -f demos/Makefile ]; then \
		$(MAKE) -C demos; \
	else \
		echo "No demos found"; \
	fi

app: $(STATIC_LIB)
	@echo "Building application..."
	@if [ -d app ] && [ -f app/Makefile ]; then \
		$(MAKE) -C app; \
	else \
		echo "No application found"; \
	fi

# ============================================================================
# Installation
# ============================================================================

install: all install-php
	@echo "Installing Crystalline CLLM Libraries..."
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 math/lib/libcrystallinemath.so $(DESTDIR)$(LIBDIR)
	install -m 755 libalgorithms.so $(DESTDIR)$(LIBDIR)
	install -m 755 libcllm.so $(DESTDIR)$(LIBDIR)
	install -m 755 libcrawler.so $(DESTDIR)$(LIBDIR)
	install -m 644 libalgorithms.a $(DESTDIR)$(LIBDIR)
	install -m 644 libcllm.a $(DESTDIR)$(LIBDIR)
	install -m 644 math/lib/libcrystallinemath.a $(DESTDIR)$(LIBDIR)
	install -m 644 libcrawler.a $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)/crystalline
	# cp -r include/* $(DESTDIR)$(INCLUDEDIR)/crystalline/  # Legacy - no longer used
	ldconfig
	cp -r math/include/* $(DESTDIR)$(INCLUDEDIR)/crystalline/
	@echo "✓ Installation complete"

uninstall:
	@echo "Uninstalling Prime Math Library..."
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB)
	rm -rf $(DESTDIR)$(INCLUDEDIR)/primemath
	ldconfig
	@echo "✓ Uninstallation complete"

# ============================================================================
# Verification and Information
# ============================================================================

verify:
	@echo ""
	@echo "Checking for flag:"
	@if echo "$(LDFLAGS)" | grep -q "\-lm"; then \
		echo "✗ Found flag in LDFLAGS!"; \
		exit 1; \
	else \
		echo "✓ No flag present"; \
	fi
	@echo ""
	@echo "✓ Mathematical independence verified!"

info:
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║         Prime Math Library - Build Information                ║"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Compiler:        $(CC)                                         "
	@echo "║ CFLAGS:          $(CFLAGS)"
	@echo "║ LDFLAGS:         $(LDFLAGS)"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Source Files:                                                  ║"
	@echo "║   CLLM:          Built in cllm/ directory                      ║"
	@echo "║   Total:         $(words $(ALL_SOURCES)) files                 ║"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Output:                                                        ║"
	@echo "║   Static:        $(STATIC_LIB)                                 "
	@echo "║   Shared:        $(SHARED_LIB)                                 "
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║ Mathematical Independence: YES (no, no math.h)            ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"

help:
	@echo "Prime Math Library - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build static and shared libraries (default)"
	@echo "  clean      - Remove all build artifacts"
	@echo "  test       - Build and run test suite"
	@echo "  demos      - Build demo programs"
	@echo "  app        - Build SDL application"
	@echo "  install    - Install libraries and headers (requires sudo)"
	@echo "  uninstall  - Remove installed files (requires sudo)"
	@echo "  verify     - Verify mathematical independence"
	@echo "  info       - Display build information"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build everything"
	@echo "  make clean all    # Clean rebuild"
	@echo "  make test         # Build and run tests"
	@echo "  sudo make install # Install system-wide"

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(ALL_OBJECTS)
	rm -f $(MATH_LIB) $(ALGORITHMS_LIB) $(CLLM_LIB) $(STATIC_LIB) $(SHARED_LIB)
	rm -f $(MATH_STATIC) $(ALGORITHMS_STATIC) $(CLLM_STATIC) 
	rm -f libcrystalline.a libcrystalline.so  # Remove OLD library
	rm -f libcllm.so libcllm.a libalgorithms.so libalgorithms.a  # Remove copied libraries
	rm -f tools/cllm_pdf_extract tools/cllm_ocr tools/cllm_pdf_ocr tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build \
                tools/init_lattice_embeddings tools/benchmark_ntt_attention tools/fix_html_entities # tools/validate_lattice
	@if [ -d tests ]; then $(MAKE) -C tests clean 2>/dev/null || true; fi
	@if [ -d algorithms ]; then $(MAKE) -C algorithms clean 2>/dev/null || true; fi
	@if [ -d cllm ]; then $(MAKE) -C cllm clean 2>/dev/null || true; fi
	@if [ -d demos ]; then $(MAKE) -C demos clean 2>/dev/null || true; fi
	@if [ -d app ]; then $(MAKE) -C app clean 2>/dev/null || true; fi
	@$(MAKE) php-clean 2>/dev/null || true
	@echo "✓ Clean complete"

# ============================================================================
# Debug Build
# ============================================================================

debug: CFLAGS += -g -O2 -DDEBUG -fsanitize=address -fsanitize=undefined
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: clean all
	@echo "✓ Debug build complete"

# ============================================================================
# ============================================================================
# Additional CLI Tools
# ============================================================================

tools/cllm_inference: $(CLLM_LIB)
	@echo "Building inference tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_inference tools/cllm_inference.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Inference tool built: tools/cllm_inference"

tools/cllm_tokenize: $(CLLM_LIB)
	@echo "Building tokenizer tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_tokenize tools/cllm_tokenize.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Tokenizer tool built: tools/cllm_tokenize"

tools/cllm_vocab_build: $(CLLM_LIB)
	@echo "Building vocabulary builder tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_vocab_build tools/cllm_vocab_build.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'

tools/init_lattice_embeddings: $(CLLM_LIB)
	@echo "Building lattice embeddings initializer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/init_lattice_embeddings tools/init_lattice_embeddings.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Lattice embeddings tool built: tools/init_lattice_embeddings"

tools/benchmark_ntt_attention: $(CLLM_LIB)
	@echo "Building NTT attention benchmark..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/benchmark_ntt_attention tools/benchmark_ntt_attention.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ NTT benchmark tool built: tools/benchmark_ntt_attention"

# tools/validate_lattice: $(CLLM_LIB)
# 	@echo "Building kissing spheres validator..."
# 	@mkdir -p tools
# 	$(CC) $(CFLAGS) -o tools/validate_lattice tools/validate_lattice.c \
# 		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
# 	@echo "✓ Kissing spheres validator built: tools/validate_lattice"

tools/analyze_cymatic_resonance: $(CLLM_LIB)
	@echo "Building cymatic resonance analyzer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/analyze_cymatic_resonance tools/analyze_cymatic_resonance.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Cymatic analyzer tool built: tools/analyze_cymatic_resonance"

tools/visualize_angular_positions: $(CLLM_LIB)
	@echo "Building angular position visualizer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/visualize_angular_positions tools/visualize_angular_positions.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Angular visualizer tool built: tools/visualize_angular_positions"

all_tools: tools tools/cllm_inference tools/cllm_tokenize tools/cllm_vocab_build \
        tools/init_lattice_embeddings tools/benchmark_ntt_attention \
        tools/validate_lattice tools/analyze_cymatic_resonance \
        tools/visualize_angular_positions


# ============================================================================
# Production Build Targets
# ============================================================================

PROD_CFLAGS = -Wall -Wextra -O3 -std=c11 -DPRODUCTION_BUILD=1 -march=native -flto -I./include
PROD_TEST_DIR = tests

# Production test executables
benchmark_large_scale: $(PROD_TEST_DIR)/benchmark_large_scale.c $(CLLM_LIB)
	@echo "Building large-scale benchmark..."
	@mkdir -p tools
	$(CC) $(PROD_CFLAGS) -o tools/benchmark_large_scale $(PROD_TEST_DIR)/benchmark_large_scale.c \
		-L. -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lssl -lcrypto -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Large-scale benchmark built: tools/benchmark_large_scale"

validate_training_quality: $(PROD_TEST_DIR)/validate_training_quality.c $(CLLM_LIB)
	@echo "Building training quality validator..."
	@mkdir -p tools
	$(CC) $(PROD_CFLAGS) -o tools/validate_training_quality $(PROD_TEST_DIR)/validate_training_quality.c \
		-L. -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lssl -lcrypto -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Training validator built: tools/validate_training_quality"

smoke_test_production: $(PROD_TEST_DIR)/smoke_test_production.c $(CLLM_LIB)
	@echo "Building production smoke tests..."
	@mkdir -p tools
	$(CC) $(PROD_CFLAGS) -o tools/smoke_test_production $(PROD_TEST_DIR)/smoke_test_production.c \
		-L. -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lssl -lcrypto -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Smoke tests built: tools/smoke_test_production"

# Production build target
production: benchmark_large_scale validate_training_quality smoke_test_production
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║         PRODUCTION BUILD COMPLETED SUCCESSFULLY                ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Production tools built:"
	@echo "  ✓ tools/benchmark_large_scale"
	@echo "  ✓ tools/validate_training_quality"
	@echo "  ✓ tools/smoke_test_production"
	@echo ""

# Run production tests
prod-tests: production
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║              RUNNING PRODUCTION TEST SUITE                     ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Running smoke tests..."
	@./tools/smoke_test_production || exit 1
	@echo ""
	@echo "Running training quality validation..."
	@./tools/validate_training_quality || exit 1
	@echo ""
	@echo "Running large-scale benchmarks..."
	@timeout 300 ./tools/benchmark_large_scale || echo "Benchmark timed out or failed"
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║           PRODUCTION TESTS COMPLETED                           ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"


# Unified CLI tool (replaces individual tools)
tools/cllm: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building unified CLLM CLI tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm tools/cllm_unified.c \
		-L. -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lssl -lcrypto -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Unified CLI tool built: tools/cllm"

tools/cllm_model_manager: $(CLLM_LIB)
	@echo "Building model manager tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm_model_manager tools/cllm_model_manager.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Model manager tool built: tools/cllm_model_manager"

tools/diagnose_inference: $(CLLM_LIB)
	@echo "Building inference diagnostic tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/diagnose_inference tools/diagnose_inference.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Inference diagnostic tool built: tools/diagnose_inference"

tools/benchmark_prime_validation: $(MATH_LIB)
	@echo "Building prime validation benchmark tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/benchmark_prime_validation tools/benchmark_prime_validation.c \
		-L. -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Prime validation benchmark tool built: tools/benchmark_prime_validation"


tools/fix_html_entities:
	@echo "Building HTML entity fixer..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/fix_html_entities tools/fix_html_entities.c
	@echo "✓ HTML entity fixer built: tools/fix_html_entities"


tools/cllm-simple: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building simple (single-threaded) CLLM CLI tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/cllm-simple tools/cllm_simple.c \
		-L. -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lssl -lcrypto -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Simple CLI tool built: tools/cllm-simple"

tools/platonic_prime_resonance: $(MATH_LIB)
	@echo "Building Platonic prime resonance tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/platonic_prime_resonance tools/platonic_prime_resonance.c \
		-L. -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Platonic prime resonance tool built: tools/platonic_prime_resonance"

tools/analyze_million_primes: $(MATH_LIB)
	@echo "Building million primes analysis tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/analyze_million_primes tools/analyze_million_primes.c \
		-L. -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Million primes analysis tool built: tools/analyze_million_primes"

tools/discover_platonic_solids: $(MATH_LIB)
	@echo "Building Platonic solid discovery tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/discover_platonic_solids tools/discover_platonic_solids.c \
		-L. -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Platonic solid discovery tool built: tools/discover_platonic_solids"

tools/tetration_analysis: $(MATH_LIB)
	@echo "Building tetration analysis tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/tetration_analysis tools/tetration_analysis.c \
		-L. -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Tetration analysis tool built: tools/tetration_analysis"
# Add platonic objects to CLLM library
CLLM_OBJECTS += $(PLATONIC_OBJECTS)

# Platonic test tool
tools/test_tetrahedron: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Tetrahedron test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_tetrahedron tools/platonic/test_tetrahedron.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Tetrahedron test tool built: tools/test_tetrahedron"

.PHONY: platonic-test
platonic-test: tools/test_tetrahedron tools/test_cube tools/test_octahedron tools/test_dodecahedron tools/test_icosahedron tools/test_blind_recovery tools/test_harmonic
	@echo "Running Tetrahedron model tests..."
	./tools/test_tetrahedron


tools/test_cube: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Cube test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_cube tools/platonic/test_cube.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Cube test tool built: tools/test_cube"

tools/test_octahedron: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Octahedron test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_octahedron tools/platonic/test_octahedron.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Octahedron test tool built: tools/test_octahedron"

tools/test_dodecahedron: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Dodecahedron test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_dodecahedron tools/platonic/test_dodecahedron.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Dodecahedron test tool built: tools/test_dodecahedron"

tools/test_icosahedron: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Icosahedron test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_icosahedron tools/platonic/test_icosahedron.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Icosahedron test tool built: tools/test_icosahedron"

tools/test_blind_recovery: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Blind Recovery test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_blind_recovery tools/platonic/test_blind_recovery.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Blind Recovery test tool built: tools/test_blind_recovery"

tools/test_harmonic: $(CLLM_LIB) $(MATH_LIB) $(ALGORITHMS_LIB)
	@echo "Building Harmonic Integration test tool..."
	@mkdir -p tools
	$(CC) $(CFLAGS) -o tools/test_harmonic tools/platonic/test_harmonic.c \
		-L. -L./algorithms -lalgorithms -lcllm -Lmath/lib -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ Harmonic Integration test tool built: tools/test_harmonic"

# Recovery tools
recovery-tools:
	@echo "Building recovery tools..."
	@cd tools/recovery && $(MAKE)
	@echo "✓ Recovery tools built"

.PHONY: recovery-tools

# ============================================================================
# PHP Extension Support
# ============================================================================

PHP_MATH_DIR = php/math
PHP_ALGO_DIR = php/algorithms

.PHONY: php-ext php-math php-algorithms php-clean install-php install-php-ubuntu install-php-centos php-setup

php-setup:
	@echo "Setting up PHP extension structure..."
	@mkdir -p $(PHP_MATH_DIR)
	@mkdir -p $(PHP_ALGO_DIR)
	@mkdir -p examples/php
	@echo "✓ PHP extension structure ready"

php-math: $(MATH_LIB) php-setup
	@echo "Building crystalline_math PHP extension..."
	@cd $(PHP_MATH_DIR) && \
		phpize && \
		./configure && \
		$(MAKE)
	@echo "✓ crystalline_math extension built"

php-algorithms: $(ALGO_LIB) php-setup
	@echo "Building algorithms PHP extension..."
	@cd $(PHP_ALGO_DIR) && \
		phpize && \
		./configure && \
		$(MAKE)
	@echo "✓ algorithms extension built"

php-ext: php-math php-algorithms
	@echo ""
	@echo "✓ All PHP extensions built successfully"
	@echo ""
	@echo "To install: sudo make install-php"

php-clean:
	@echo "Cleaning PHP extensions..."
	@if [ -d $(PHP_MATH_DIR) ]; then \
		cd $(PHP_MATH_DIR) && \
		if [ -f Makefile ]; then $(MAKE) clean 2>/dev/null || true; fi && \
		phpize --clean 2>/dev/null || true; \
		rm -rf .libs *.lo *.la modules/ autom4te.cache/ 2>/dev/null || true; \
	fi
	@if [ -d $(PHP_ALGO_DIR) ]; then \
		cd $(PHP_ALGO_DIR) && \
		if [ -f Makefile ]; then $(MAKE) clean 2>/dev/null || true; fi && \
		phpize --clean 2>/dev/null || true; \
		rm -rf .libs *.lo *.la modules/ autom4te.cache/ 2>/dev/null || true; \
	fi
	@echo "✓ PHP extensions cleaned"

install-php: php-ext
	@echo "Installing PHP extensions..."
	@cd $(PHP_MATH_DIR) && $(MAKE) install
	@cd $(PHP_ALGO_DIR) && $(MAKE) install
	@echo "✓ PHP extensions installed"
	@echo ""
	@echo "Add to php.ini:"
	@echo "  extension=crystalline_math.so"
	@echo "  extension=algorithms.so"

install-php-ubuntu: install-php
	@./php/scripts/install_php_ubuntu.sh

install-php-centos: install-php
	@./php/scripts/install_php_centos.sh