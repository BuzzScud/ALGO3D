# OBJECTIVE 28: Geometric Recovery Algorithm - Independent Makefile
# This Makefile builds the complete geometric recovery system independently

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I./include -I../../include
LDFLAGS = -L../../crystalline -lcrystalline -lm -lssl -lcrypto -lpthread

# Directories
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
BUILD_DIR = build
LIB_DIR = lib

# Output
LIB_NAME = libgeometric_recovery.a
LIB_SO = libgeometric_recovery.so

# Source files
SOURCES = \
	$(SRC_DIR)/geometric_recovery_complete.c \
	$(SRC_DIR)/geometric_utils.c \
	$(SRC_DIR)/oscillation_vector.c \
	$(SRC_DIR)/q_validation.c \
	$(SRC_DIR)/quadrant_polarity.c \
	$(SRC_DIR)/shared_geometry.c \
	$(SRC_DIR)/platonic_model_core.c \
	$(SRC_DIR)/platonic_model_oscillations.c \
	$(SRC_DIR)/platonic_model_recovery.c \
	$(SRC_DIR)/platonic_model_scaling.c \
	$(SRC_DIR)/platonic_model_persistence.c \
	$(SRC_DIR)/tetration.c \
	$(SRC_DIR)/prime_float_math.c \
	$(SRC_DIR)/iterative_recovery.c \
	$(SRC_DIR)/iterative_recovery_v2.c \
        $(SRC_DIR)/recursive_recovery.c \
        $(SRC_DIR)/clock_recovery.c \
        $(SRC_DIR)/platonic_solids.c \
        $(SRC_DIR)/k_recovery_enhanced.c \
        $(SRC_DIR)/spherical_recovery.c \
        $(SRC_DIR)/search_recovery.c \
        $(SRC_DIR)/search_recovery_v2.c \
        $(SRC_DIR)/anchor_grid_24.c \
        $(SRC_DIR)/search_recovery_v3.c \
        $(SRC_DIR)/search_recovery_v4.c \
        $(SRC_DIR)/prime_rainbow_recovery.c \
        $(SRC_DIR)/geometric_anchors.c \
        $(SRC_DIR)/anchor_tracking.c \
        $(SRC_DIR)/ecdsa_test_generator.c \
        $(SRC_DIR)/ecdsa_sample_loader.c \
        $(SRC_DIR)/integrated_recovery.c \
        $(SRC_DIR)/multi_torus_tracker.c \
        $(SRC_DIR)/oscillation_decomposition.c \
        $(SRC_DIR)/torus_analysis.c \
        $(SRC_DIR)/prime_factor_extraction.c \
        $(SRC_DIR)/clock_lattice_integration.c \
        $(SRC_DIR)/search_recovery_v5.c \
        $(SRC_DIR)/oscillation_detection.c \
        $(SRC_DIR)/crystal_abacus.c \
        $(SRC_DIR)/ecdlp_integration.c \
        $(SRC_DIR)/g_triangulation.c \
        $(SRC_DIR)/harmonic_folding.c \
        $(SRC_DIR)/kissing_spheres.c \
        $(SRC_DIR)/micro_model.c \
        $(SRC_DIR)/multi_scalar_analysis.c \
        $(SRC_DIR)/plateau_detection.c \
        $(SRC_DIR)/recursive_search.c \
        $(SRC_DIR)/tetration_attractors.c \
        $(SRC_DIR)/full_pipeline.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test programs
TEST_SOURCES = \
	$(TEST_DIR)/test_iterative_recovery_v2.c \
	$(TEST_DIR)/test_geometric_recovery.c \
	$(TEST_DIR)/test_torus_recovery_v2.c

TEST_BINS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)

# Default target
all: directories lib tests

# Create directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)

# Build static library
lib: $(LIB_DIR)/$(LIB_NAME)

$(LIB_DIR)/$(LIB_NAME): $(OBJECTS)
	@echo "Creating static library: $@"
	ar rcs $@ $^
	@echo "Static library created successfully"

# Build shared library
shared: $(LIB_DIR)/$(LIB_SO)

$(LIB_DIR)/$(LIB_SO): $(OBJECTS)
	@echo "Creating shared library: $@"
	$(CC) -shared -o $@ $^ $(LDFLAGS)
	@echo "Shared library created successfully"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Build test programs
tests: $(TEST_BINS)

$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(LIB_DIR)/$(LIB_NAME)
	@echo "Building test: $@"
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS) -o $@

# Run tests
test: tests
	@echo "Running test_iterative_recovery_v2..."
	@$(BUILD_DIR)/test_iterative_recovery_v2 || true
	@echo ""
	@echo "Running test_geometric_recovery..."
	@$(BUILD_DIR)/test_geometric_recovery || true
	@echo ""
	@echo "Running test_torus_recovery_v2..."
	@$(BUILD_DIR)/test_torus_recovery_v2 || true

# Quick test (just iterative recovery)
quicktest: $(BUILD_DIR)/test_iterative_recovery_v2
	@echo "Running quick test..."
	@$(BUILD_DIR)/test_iterative_recovery_v2

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
	@echo "Clean complete"

# Install library system-wide (requires sudo)
install: lib
	@echo "Installing library to /usr/local/lib..."
	sudo cp $(LIB_DIR)/$(LIB_NAME) /usr/local/lib/
	@echo "Installing headers to /usr/local/include..."
	sudo mkdir -p /usr/local/include/geometric_recovery
	sudo cp $(INC_DIR)/*.h /usr/local/include/geometric_recovery/
	sudo ldconfig
	@echo "Installation complete"

# Uninstall library
uninstall:
	@echo "Uninstalling library..."
	sudo rm -f /usr/local/lib/$(LIB_NAME)
	sudo rm -rf /usr/local/include/geometric_recovery
	sudo ldconfig
	@echo "Uninstall complete"

# Show file sizes
sizes: lib
	@echo "Library sizes:"
	@ls -lh $(LIB_DIR)/$(LIB_NAME)
	@echo ""
	@echo "Object file sizes:"
	@ls -lh $(BUILD_DIR)/*.o | awk '{print $$9, $$5}'

# Count lines of code
loc:
	@echo "Lines of code:"
	@echo "Source files:"
	@wc -l $(SOURCES) | tail -1
	@echo ""
	@echo "Header files:"
	@wc -l $(INC_DIR)/*.h | tail -1
	@echo ""
	@echo "Test files:"
	@wc -l $(TEST_SOURCES) | tail -1
	@echo ""
	@echo "Total:"
	@cat $(SOURCES) $(INC_DIR)/*.h $(TEST_SOURCES) | wc -l

# Generate documentation
docs:
	@echo "Generating documentation..."
	@echo "See README.md, ARCHITECTURE.md, and ALGORITHM_EXPLAINED.md"
	@echo "Documentation is already complete in docs/ directory"

# Check for common issues
check:
	@echo "Checking for common issues..."
	@echo "Checking for math.h usage (should use prime_float_math.h):"
	@grep -n "include <math.h>" $(SOURCES) || echo "  ✓ No math.h found"
	@echo ""
	@echo "Checking for memory leaks (requires valgrind):"
	@which valgrind > /dev/null && echo "  Run: make valgrind" || echo "  Install valgrind to check for leaks"
	@echo ""
	@echo "Checking for undefined behavior (requires ubsan):"
	@echo "  Rebuild with: CFLAGS += -fsanitize=undefined"

# Memory leak check (requires valgrind)
valgrind: $(BUILD_DIR)/test_iterative_recovery_v2
	@echo "Running valgrind memory leak check..."
	valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --verbose \
		$(BUILD_DIR)/test_iterative_recovery_v2

# Performance profiling (requires gprof)
profile: CFLAGS += -pg
profile: clean all
	@echo "Running with profiling enabled..."
	@$(BUILD_DIR)/test_iterative_recovery_v2
	@gprof $(BUILD_DIR)/test_iterative_recovery_v2 gmon.out > profile.txt
	@echo "Profile saved to profile.txt"

# Benchmark all tests
benchmark: tests
	@echo "Benchmarking all tests..."
	@echo "Test 1: Iterative Recovery V2"
	@time $(BUILD_DIR)/test_iterative_recovery_v2
	@echo ""
	@echo "Test 2: Geometric Recovery"
	@time $(BUILD_DIR)/test_geometric_recovery
	@echo ""
	@echo "Test 3: Torus Recovery V2"
	@time $(BUILD_DIR)/test_torus_recovery_v2

# Help
help:
	@echo "OBJECTIVE 28: Geometric Recovery Algorithm - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build library and tests (default)"
	@echo "  lib          - Build static library only"
	@echo "  shared       - Build shared library"
	@echo "  tests        - Build test programs"
	@echo "  test         - Run all tests"
	@echo "  quicktest    - Run quick test only"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install library system-wide (requires sudo)"
	@echo "  uninstall    - Uninstall library"
	@echo "  sizes        - Show file sizes"
	@echo "  loc          - Count lines of code"
	@echo "  docs         - Generate documentation"
	@echo "  check        - Check for common issues"
	@echo "  valgrind     - Run memory leak check"
	@echo "  profile      - Run performance profiling"
	@echo "  benchmark    - Benchmark all tests"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build everything"
	@echo "  make test         # Build and run tests"
	@echo "  make quicktest    # Quick test"
	@echo "  make clean all    # Clean rebuild"
	@echo "  make install      # Install system-wide"

.PHONY: all directories lib shared tests test quicktest clean install uninstall \
        sizes loc docs check valgrind profile benchmark help

# Clock recovery test
$(BUILD_DIR)/test_clock_mapping: $(TEST_DIR)/test_clock_mapping.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-clock
test-clock: $(BUILD_DIR)/test_clock_mapping
	@echo "Running clock mapping tests..."
	@$(BUILD_DIR)/test_clock_mapping

# Platonic solids test
$(BUILD_DIR)/test_platonic_solids: $(TEST_DIR)/test_platonic_solids.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-platonic
test-platonic: $(BUILD_DIR)/test_platonic_solids
	@echo "Running Platonic solids tests..."
	@$(BUILD_DIR)/test_platonic_solids

# Integrated recovery test
$(BUILD_DIR)/test_integrated_recovery: $(TEST_DIR)/test_integrated_recovery.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-integrated
test-integrated: $(BUILD_DIR)/test_integrated_recovery
	@echo "Running integrated recovery tests..."
	@$(BUILD_DIR)/test_integrated_recovery

.PHONY: test-all
test-all: test-clock test-platonic test-integrated
	@echo ""
	@echo "All tests completed!"

# Enhanced k recovery test
$(BUILD_DIR)/test_k_recovery_enhanced: $(TEST_DIR)/test_k_recovery_enhanced.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-k-recovery
test-k-recovery: $(BUILD_DIR)/test_k_recovery_enhanced
	@echo "Running enhanced k recovery tests..."
	@$(BUILD_DIR)/test_k_recovery_enhanced

# Deep recovery analysis test
$(BUILD_DIR)/test_deep_recovery_analysis: $(TEST_DIR)/test_deep_recovery_analysis.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-deep-analysis
test-deep-analysis: $(BUILD_DIR)/test_deep_recovery_analysis
	@echo "Running deep recovery analysis..."
	@$(BUILD_DIR)/test_deep_recovery_analysis

# Quadrant analysis test
$(BUILD_DIR)/test_quadrant_analysis: $(TEST_DIR)/test_quadrant_analysis.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-quadrant
test-quadrant: $(BUILD_DIR)/test_quadrant_analysis
	@echo "Running quadrant analysis..."
	@$(BUILD_DIR)/test_quadrant_analysis

# Spherical recovery test
$(BUILD_DIR)/test_spherical_recovery: $(TEST_DIR)/test_spherical_recovery.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-spherical
test-spherical: $(BUILD_DIR)/test_spherical_recovery
	@echo "Running spherical recovery test..."
	@$(BUILD_DIR)/test_spherical_recovery

# Search recovery test
$(BUILD_DIR)/test_search_recovery: $(TEST_DIR)/test_search_recovery.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-search
test-search: $(BUILD_DIR)/test_search_recovery
	@echo "Running search-based recovery test..."
	@$(BUILD_DIR)/test_search_recovery

# Geometric anchors test
$(BUILD_DIR)/test_geometric_anchors: $(TEST_DIR)/test_geometric_anchors.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-geometric
test-geometric: $(BUILD_DIR)/test_geometric_anchors
	@echo "Running geometric anchor system test..."
	@$(BUILD_DIR)/test_geometric_anchors

# Shared vertex tolerance test
$(BUILD_DIR)/test_shared_vertex_tolerance: $(TEST_DIR)/test_shared_vertex_tolerance.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-tolerance
test-tolerance: $(BUILD_DIR)/test_shared_vertex_tolerance
	@echo "Running shared vertex tolerance analysis..."
	@$(BUILD_DIR)/test_shared_vertex_tolerance

# Search recovery v2 test
$(BUILD_DIR)/test_search_recovery_v2: $(TEST_DIR)/test_search_recovery_v2.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-search-v2
test-search-v2: $(BUILD_DIR)/test_search_recovery_v2
	@echo "Running enhanced search recovery test (v2)..."
	@$(BUILD_DIR)/test_search_recovery_v2

# Search recovery v3 test (24-anchor grid)
$(BUILD_DIR)/test_search_recovery_v3: $(TEST_DIR)/test_search_recovery_v3.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-search-v3
test-search-v3: $(BUILD_DIR)/test_search_recovery_v3
	@echo "Running 24-anchor grid enhancement test (v3)..."
	@$(BUILD_DIR)/test_search_recovery_v3

# Prime rainbow recovery test
$(BUILD_DIR)/test_prime_rainbow_recovery: $(TEST_DIR)/test_prime_rainbow_recovery.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-prime-rainbow
test-prime-rainbow: $(BUILD_DIR)/test_prime_rainbow_recovery
	@echo "Running prime rainbow recovery test..."
	@$(BUILD_DIR)/test_prime_rainbow_recovery

# Search recovery v4 test (direct k-based anchors)
$(BUILD_DIR)/test_search_recovery_v4: $(TEST_DIR)/test_search_recovery_v4.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-search-v4
test-search-v4: $(BUILD_DIR)/test_search_recovery_v4
	@echo "Running direct k-based anchor test (v4)..."
	@$(BUILD_DIR)/test_search_recovery_v4

# Deep success pattern analysis
$(BUILD_DIR)/test_success_pattern_deep_analysis: $(TEST_DIR)/test_success_pattern_deep_analysis.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-success-deep
test-success-deep: $(BUILD_DIR)/test_success_pattern_deep_analysis
	@echo "Running deep success pattern analysis..."
	@$(BUILD_DIR)/test_success_pattern_deep_analysis

# Search recovery v5 (radius-aware)
$(BUILD_DIR)/search_recovery_v5.o: $(SRC_DIR)/search_recovery_v5.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_search_recovery_v5: $(TEST_DIR)/test_search_recovery_v5.c $(LIB_DIR)/$(LIB_NAME) $(BUILD_DIR)/search_recovery_v5.o
	$(CC) $(CFLAGS) -o $@ $< $(BUILD_DIR)/search_recovery_v5.o -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS)

.PHONY: test-search-v5
test-search-v5: $(BUILD_DIR)/test_search_recovery_v5
	@echo "Running radius-aware search test (v5)..."
	@$(BUILD_DIR)/test_search_recovery_v5

# ═══════════════════════════════════════════════════════════════════════
#                     PHASE 1: FOUNDATION (NEW)
# ═══════════════════════════════════════════════════════════════════════

# Crystal Abacus (prime generation with lattice)
$(BUILD_DIR)/crystal_abacus.o: $(SRC_DIR)/crystal_abacus.c
	$(CC) $(CFLAGS) -c $< -o $@

# Kissing Spheres (recursive hierarchy)
$(BUILD_DIR)/kissing_spheres.o: $(SRC_DIR)/kissing_spheres.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phase 1 Foundation Test
$(BUILD_DIR)/test_phase1_foundation: $(TEST_DIR)/test_phase1_foundation.c $(BUILD_DIR)/crystal_abacus.o $(BUILD_DIR)/kissing_spheres.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-phase1
test-phase1: $(BUILD_DIR)/test_phase1_foundation
	@echo "Running Phase 1 Foundation Tests..."
	@$(BUILD_DIR)/test_phase1_foundation


# Link with crystalline prime math library
PRIME_MATH_OBJS = ../../build/prime_float_math.o

$(BUILD_DIR)/test_phase1_foundation: $(TEST_DIR)/test_phase1_foundation.c $(BUILD_DIR)/crystal_abacus.o $(BUILD_DIR)/kissing_spheres.o
	@echo "Building with crystalline prime math..."
	@if [ ! -f ../../build/prime_float_math.o ]; then \
		echo "Building prime_float_math.o..."; \
		cd ../.. && make build/prime_float_math.o; \
	fi
	$(CC) $(CFLAGS) -o $@ $< $(BUILD_DIR)/crystal_abacus.o $(BUILD_DIR)/kissing_spheres.o ../../build/prime_float_math.o $(LDFLAGS)


# Anchor tracking test
$(BUILD_DIR)/test_anchor_tracking: $(TEST_DIR)/test_anchor_tracking.c $(BUILD_DIR)/anchor_tracking.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building anchor tracking test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-anchor-tracking
test-anchor-tracking: $(BUILD_DIR)/test_anchor_tracking
	@echo "Running Anchor Tracking Tests..."
	@$(BUILD_DIR)/test_anchor_tracking

# ECDSA sample loader test
$(BUILD_DIR)/test_ecdsa_samples: $(TEST_DIR)/test_ecdsa_samples.c $(BUILD_DIR)/ecdsa_sample_loader.o $(BUILD_DIR)/anchor_tracking.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building ECDSA sample loader test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-ecdsa-samples
test-ecdsa-samples: $(BUILD_DIR)/test_ecdsa_samples
	@echo "Running ECDSA Sample Loader Tests..."
	@$(BUILD_DIR)/test_ecdsa_samples

# Integrated recovery test v2
$(BUILD_DIR)/test_integrated_recovery_v2: $(TEST_DIR)/test_integrated_recovery_v2.c $(BUILD_DIR)/integrated_recovery.o $(BUILD_DIR)/ecdsa_sample_loader.o $(BUILD_DIR)/anchor_tracking.o $(BUILD_DIR)/geometric_anchors.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building integrated recovery v2 test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-integrated-v2
test-integrated-v2: $(BUILD_DIR)/test_integrated_recovery_v2
	@echo "Running Integrated Recovery V2 Tests..."
	@$(BUILD_DIR)/test_integrated_recovery_v2


# G Triangulation Test
$(BUILD_DIR)/test_g_triangulation: $(TEST_DIR)/test_g_triangulation.c $(BUILD_DIR)/prime_float_math.o
	@echo "Building G triangulation test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-g-triangulation
test-g-triangulation: $(BUILD_DIR)/test_g_triangulation
	@echo "Running G Triangulation Tests..."
	@$(BUILD_DIR)/test_g_triangulation



# Iterative Refinement Test
$(BUILD_DIR)/test_iterative_refinement: $(TEST_DIR)/test_iterative_refinement.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building iterative refinement test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-iterative-refinement
test-iterative-refinement: $(BUILD_DIR)/test_iterative_refinement
	@echo "Running Iterative Refinement Tests..."
	@$(BUILD_DIR)/test_iterative_refinement



# Convergence Analysis Test
$(BUILD_DIR)/test_convergence_analysis: $(TEST_DIR)/test_convergence_analysis.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building convergence analysis test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-convergence
test-convergence: $(BUILD_DIR)/test_convergence_analysis
	@echo "Running Convergence Analysis Tests..."
	@$(BUILD_DIR)/test_convergence_analysis



# Torus Analysis Test
$(BUILD_DIR)/test_torus_analysis: $(TEST_DIR)/test_torus_analysis.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/plateau_detection.o $(BUILD_DIR)/torus_analysis.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building torus analysis test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-torus
test-torus: $(BUILD_DIR)/test_torus_analysis
	@echo "Running Torus Analysis Tests..."
	@$(BUILD_DIR)/test_torus_analysis



# Dual Scalar Decomposition Test
$(BUILD_DIR)/test_dual_scalar_decomposition: $(TEST_DIR)/test_dual_scalar_decomposition.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/plateau_detection.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building dual scalar decomposition test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-dual-scalar
test-dual-scalar: $(BUILD_DIR)/test_dual_scalar_decomposition
	@echo "Running Dual Scalar Decomposition Tests..."
	@$(BUILD_DIR)/test_dual_scalar_decomposition


# Extended iteration test (2000 iterations to fix period 2 issue)
$(BUILD_DIR)/test_extended_iterations: $(TEST_DIR)/test_extended_iterations.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/plateau_detection.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/ecdsa_sample_loader.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building extended iteration test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-extended
test-extended: $(BUILD_DIR)/test_extended_iterations
	@echo "Running Extended Iteration Tests (2000 iterations)..."
	@$(BUILD_DIR)/test_extended_iterations

# Harmonic folding
$(BUILD_DIR)/harmonic_folding.o: $(SRC_DIR)/harmonic_folding.c
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Harmonic folding test
$(BUILD_DIR)/test_harmonic_folding: $(TEST_DIR)/test_harmonic_folding.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/plateau_detection.o $(BUILD_DIR)/harmonic_folding.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building harmonic folding test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-harmonic
test-harmonic: $(BUILD_DIR)/test_harmonic_folding
	@echo "Running Harmonic Folding Tests..."
	@$(BUILD_DIR)/test_harmonic_folding

# Comprehensive torus analysis test (up to 20 tori)
$(BUILD_DIR)/test_comprehensive_torus: $(TEST_DIR)/test_comprehensive_torus_analysis.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/plateau_detection.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/harmonic_folding.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building comprehensive torus analysis test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-comprehensive
test-comprehensive: $(BUILD_DIR)/test_comprehensive_torus
	@echo "Running Comprehensive Torus Analysis (up to 20 tori)..."
	@$(BUILD_DIR)/test_comprehensive_torus

# Torus intersection

# Torus intersection test
$(BUILD_DIR)/test_torus_intersection: $(TEST_DIR)/test_torus_intersection.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building torus intersection test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-intersection
test-intersection: $(BUILD_DIR)/test_torus_intersection
	@echo "Running Torus Intersection Tests..."
	@$(BUILD_DIR)/test_torus_intersection

# Per-sample torus analysis test
$(BUILD_DIR)/test_per_sample_torus: $(TEST_DIR)/test_per_sample_torus.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building per-sample torus analysis test..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test-per-sample
test-per-sample: $(BUILD_DIR)/test_per_sample_torus
	@echo "Running Per-Sample Torus Analysis..."
	@$(BUILD_DIR)/test_per_sample_torus

# Multi-sample intersection test
$(BUILD_DIR)/test_multi_sample_intersection: $(TEST_DIR)/test_multi_sample_intersection.c $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building multi-sample intersection test..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test-multi-sample: $(BUILD_DIR)/test_multi_sample_intersection
	@echo "Running Multi-Sample Intersection Analysis..."
	@$(BUILD_DIR)/test_multi_sample_intersection

# Platonic solid verification test
$(BUILD_DIR)/test_platonic_solid_verification: $(TEST_DIR)/test_platonic_solid_verification.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building Platonic solid verification test..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test-platonic: $(BUILD_DIR)/test_platonic_solid_verification
	@echo "Running Platonic Solid Verification Test..."
	@$(BUILD_DIR)/test_platonic_solid_verification

# Extract p and q test
$(BUILD_DIR)/test_extract_p_q: $(TEST_DIR)/test_extract_p_q.c $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building p/q extraction test..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test-extract-pq: $(BUILD_DIR)/test_extract_p_q
	@echo "Running p/q Extraction Test..."
	@$(BUILD_DIR)/test_extract_p_q

# Comprehensive p/q extraction test
$(BUILD_DIR)/test_comprehensive_pq_extraction: $(TEST_DIR)/test_comprehensive_pq_extraction.c $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o
	@echo "Building comprehensive p/q extraction test..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test-comprehensive-pq: $(BUILD_DIR)/test_comprehensive_pq_extraction
	@echo "Running Comprehensive p/q Extraction Test..."
	@$(BUILD_DIR)/test_comprehensive_pq_extraction

# Clock lattice p/q visualization test
$(BUILD_DIR)/test_clock_lattice_pq_visualization: $(TEST_DIR)/test_clock_lattice_pq_visualization.c $(BUILD_DIR)/prime_float_math.o
	@echo "Building clock lattice p/q visualization test..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

test-clock-pq: $(BUILD_DIR)/test_clock_lattice_pq_visualization
	@echo "Running Clock Lattice p/q Visualization Test..."
	@$(BUILD_DIR)/test_clock_lattice_pq_visualization

# Clock lattice visualization test (Phase 3)

# Clock lattice visualization test (Phase 3)
$(BUILD_DIR)/test_clock_lattice_visualization: $(TEST_DIR)/test_clock_lattice_visualization.c $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building clock lattice visualization test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-clock-viz: $(BUILD_DIR)/test_clock_lattice_visualization
	@echo "Running Clock Lattice Visualization Test..."
	@$(BUILD_DIR)/test_clock_lattice_visualization

# G refinement with clock lattice test (Phase 4)
$(BUILD_DIR)/test_g_refinement_with_clock: $(TEST_DIR)/test_g_refinement_with_clock.c $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building G refinement with clock lattice test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-g-refine: $(BUILD_DIR)/test_g_refinement_with_clock
	@echo "Running G Refinement with Clock Lattice Test..."
	@$(BUILD_DIR)/test_g_refinement_with_clock

# Map 20 tori to clock lattice test (Revised Phase 4)
$(BUILD_DIR)/test_map_20_tori_to_clock: $(TEST_DIR)/test_map_20_tori_to_clock.c $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building map 20 tori to clock test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-map-tori: $(BUILD_DIR)/test_map_20_tori_to_clock
	@echo "Running Map 20 Tori to Clock Test..."
	@$(BUILD_DIR)/test_map_20_tori_to_clock

# Micro-model test (Phase 5)
$(BUILD_DIR)/test_micro_model: $(TEST_DIR)/test_micro_model.c $(BUILD_DIR)/micro_model.o $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building micro-model test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

# Micro-model object
$(BUILD_DIR)/micro_model.o: $(SRC_DIR)/micro_model.c
	@echo "Compiling micro_model.c..."
	@$(CC) $(CFLAGS) -I../../include -c $< -o $@

test-micro-model: $(BUILD_DIR)/test_micro_model
	@echo "Running Micro-Model Test..."
	@$(BUILD_DIR)/test_micro_model

# Real ECDSA samples test (Task 7)
$(BUILD_DIR)/test_real_ecdsa_samples: $(TEST_DIR)/test_real_ecdsa_samples.c $(BUILD_DIR)/micro_model.o $(BUILD_DIR)/g_triangulation.o $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building real ECDSA samples test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-real-ecdsa: $(BUILD_DIR)/test_real_ecdsa_samples
	@echo "Running Real ECDSA Samples Test..."
	@$(BUILD_DIR)/test_real_ecdsa_samples

# 64-bit and 128-bit validation test
$(BUILD_DIR)/test_64_128_bit_validation: $(TEST_DIR)/test_64_128_bit_validation.c $(BUILD_DIR)/multi_torus_tracker.o $(BUILD_DIR)/oscillation_decomposition.o $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building 64/128-bit validation test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-64-128: $(BUILD_DIR)/test_64_128_bit_validation
	@echo "Running 64-bit and 128-bit Validation Test..."
	@$(BUILD_DIR)/test_64_128_bit_validation

# Large prime clock mapping test
$(BUILD_DIR)/test_large_prime_clock_mapping: $(TEST_DIR)/test_large_prime_clock_mapping.c $(BUILD_DIR)/prime_float_math.o ../../src/geometry/clock_lattice.o
	@echo "Building large prime clock mapping test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-large-primes: $(BUILD_DIR)/test_large_prime_clock_mapping
	@echo "Running Large Prime Clock Mapping Test..."
	@$(BUILD_DIR)/test_large_prime_clock_mapping

# Real torus data validation test
$(BUILD_DIR)/test_real_torus_data_validation: $(TEST_DIR)/test_real_torus_data_validation.c $(BUILD_DIR)/prime_float_math.o
	@echo "Building real torus data validation test..."
	@$(CC) $(CFLAGS) -I../../include -o $@ $^ $(LDFLAGS)

test-real-torus: $(BUILD_DIR)/test_real_torus_data_validation
	@echo "Running Real Torus Data Validation Test..."
	@$(BUILD_DIR)/test_real_torus_data_validation

# Additional sources for comprehensive library
SOURCES += \
	$(SRC_DIR)/micro_model.c \
	$(SRC_DIR)/g_triangulation.c \
	$(SRC_DIR)/multi_torus_tracker.c \
	$(SRC_DIR)/oscillation_decomposition.c \
	$(SRC_DIR)/plateau_detection.c \
	$(SRC_DIR)/harmonic_folding.c \
	$(SRC_DIR)/torus_analysis.c \
	$(SRC_DIR)/oscillation_detection.c \
	$(SRC_DIR)/multi_scalar_analysis.c

# Phase 6: Full Pipeline Test
$(BUILD_DIR)/test_full_pipeline: $(TEST_DIR)/test_full_pipeline.c $(LIB_DIR)/$(LIB_NAME)
	@echo "Building full pipeline test: $@"
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -lgeometric_recovery $(LDFLAGS) -o $@

test-full-pipeline: $(BUILD_DIR)/test_full_pipeline
	@echo ""
	@echo "╔══════════════════════════════════════════════════════════╗"
	@echo "║  Running Full Pipeline Test                              ║"
	@echo "╚══════════════════════════════════════════════════════════╝"
	@$(BUILD_DIR)/test_full_pipeline

