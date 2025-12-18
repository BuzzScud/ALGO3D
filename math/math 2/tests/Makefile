# Test Suite Makefile for Crystalline CLLM

CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -I../cllm/include -I../algorithms/include -I../math/include -mavx2 -mfma
LDFLAGS = -L../cllm -L../algorithms -L../math/lib -Wl,--start-group -lcllm -lalgorithms -lcrystallinemath -Wl,--end-group -lm -lpthread -Wl,-rpath,'$$ORIGIN/../cllm':'$$ORIGIN/../algorithms':'$$ORIGIN/../math/lib'

# Test directories
UNIT_DIR = unit
INTEGRATION_DIR = integration
PERFORMANCE_DIR = performance
VALIDATION_DIR = validation

# Unit tests
UNIT_TESTS = \
	$(UNIT_DIR)/test_softmax_backward \
	$(UNIT_DIR)/test_attention_cache

# Integration tests
INTEGRATION_TESTS = \
	$(INTEGRATION_DIR)/test_forward_backward \
	$(INTEGRATION_DIR)/test_lr_scheduling

# Performance tests
PERFORMANCE_TESTS = \
	$(PERFORMANCE_DIR)/benchmark_training_speed

# Validation tests
VALIDATION_TESTS = \
	$(VALIDATION_DIR)/test_numerical_gradients

# Mathematical integration tests
MATH_TESTS = test_mathematical_integration

# Diagnostic tests (moved from tools/)
DIAGNOSTIC_TESTS = \
        test_simple_init \
        test_token_init \
        test_hierarchical \
        quick_inference_check \
        profile_lattice

# Kissing spheres tests
KISSING_SPHERES_TESTS = \
        test_lattice_visualization \
        test_lattice_integration

# Phase 2 tests (NEW - file I/O and training functions)
PHASE2_TESTS = \
        test_file_io \
        test_training_functions

# Phase 3 tests (NEW - CrystallineAbacus integration)
PHASE3_TESTS = \
        test_abacus_matrix

# 88D Training tests
TRAINING_88D_TESTS = \
        test_gradient_accumulation \
        test_training_loop_88d

# Geometric Matrix tests (NEW)
GEOMETRIC_TESTS = \
        test_geometric_matrix_simple \
        test_geometric_integration

# All tests
ALL_TESTS = $(UNIT_TESTS) $(INTEGRATION_TESTS) $(PERFORMANCE_TESTS) $(VALIDATION_TESTS) $(MATH_TESTS) $(DIAGNOSTIC_TESTS) $(KISSING_SPHERES_TESTS) $(PHASE2_TESTS) $(PHASE3_TESTS) $(TRAINING_88D_TESTS) $(GEOMETRIC_TESTS)

.PHONY: all unit integration performance validation diagnostic kissing-spheres training-88d geometric clean run-unit run-integration run-performance run-validation run-diagnostic run-kissing-spheres run-training-88d run-geometric run-all

all: unit integration diagnostic kissing-spheres training-88d geometric

unit: $(UNIT_TESTS)

integration: $(INTEGRATION_TESTS)

performance: $(PERFORMANCE_TESTS)

validation: $(VALIDATION_TESTS)

diagnostic: $(DIAGNOSTIC_TESTS)

kissing-spheres: $(KISSING_SPHERES_TESTS)

training-88d: $(TRAINING_88D_TESTS)

geometric: $(GEOMETRIC_TESTS)

# Unit test compilation
$(UNIT_DIR)/test_softmax_backward: $(UNIT_DIR)/test_softmax_backward.c
	@echo "Building unit test: test_softmax_backward..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_softmax_backward built"

$(UNIT_DIR)/test_attention_cache: $(UNIT_DIR)/test_attention_cache.c
	@echo "Building unit test: test_attention_cache..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_attention_cache built"

# Integration test compilation
$(INTEGRATION_DIR)/test_forward_backward: $(INTEGRATION_DIR)/test_forward_backward.c
	@echo "Building integration test: test_forward_backward..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_forward_backward built"

$(INTEGRATION_DIR)/test_lr_scheduling: $(INTEGRATION_DIR)/test_lr_scheduling.c
	@echo "Building integration test: test_lr_scheduling..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_lr_scheduling built"

# Performance test compilation
$(PERFORMANCE_DIR)/benchmark_training_speed: $(PERFORMANCE_DIR)/benchmark_training_speed.c
	@echo "Building performance test: benchmark_training_speed..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ benchmark_training_speed built"

# Validation test compilation
$(VALIDATION_DIR)/test_numerical_gradients: $(VALIDATION_DIR)/test_numerical_gradients.c
	@echo "Building validation test: test_numerical_gradients..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_numerical_gradients built"

# Run tests
run-unit: unit
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Unit Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(UNIT_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All unit tests completed"

run-integration: integration
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Integration Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(INTEGRATION_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All integration tests completed"

run-performance: performance
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Performance Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(PERFORMANCE_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All performance tests completed"

run-validation: validation
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "Running Validation Tests"
	@echo "═══════════════════════════════════════════════════════════"
	@for test in $(VALIDATION_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All validation tests completed"

run-all: all
	@$(MAKE) run-unit
	@$(MAKE) run-integration

clean:
	@echo "Cleaning test executables..."
	@rm -f $(ALL_TESTS)
	@echo "✓ Tests cleaned"

help:
	@echo "Crystalline CLLM Test Suite"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build unit and integration tests (default)"
	@echo "  unit             - Build unit tests only"
	@echo "  integration      - Build integration tests only"
	@echo "  performance      - Build performance tests only"
	@echo "  validation       - Build validation tests only"
	@echo ""
	@echo "  run-unit         - Run unit tests"
	@echo "  run-integration  - Run integration tests"
	@echo "  run-performance  - Run performance tests"
	@echo "  run-validation   - Run validation tests"
	@echo "  run-all          - Run all tests"
	@echo ""
	@echo "  clean            - Remove test executables"
	@echo "  help             - Show this help message"
# Mathematical integration test
test_mathematical_integration: test_mathematical_integration.c
	@echo "Building mathematical integration test..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Mathematical integration test built"

run-math: test_mathematical_integration
	@echo ""
	@echo "Running mathematical integration tests..."
	@echo ""
	@./test_mathematical_integration
	@echo ""
	@echo "✓ Mathematical integration tests completed"

# Diagnostic tests (moved from tools/)
test_simple_init: test_simple_init.c
	@echo "Building test_simple_init..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_simple_init built"

test_token_init: test_token_init.c
	@echo "Building test_token_init..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_token_init built"

test_hierarchical: test_hierarchical.c
	@echo "Building test_hierarchical..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_hierarchical built"

quick_inference_check: quick_inference_check.c
	@echo "Building quick_inference_check..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ quick_inference_check built"

profile_lattice: profile_lattice.c
	@echo "Building profile_lattice..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ profile_lattice built"

run-diagnostic: $(DIAGNOSTIC_TESTS)
	@echo ""
	@echo "Running diagnostic tests..."
	@echo ""
	@for test in $(DIAGNOSTIC_TESTS); do \
		echo "Running $$test..."; \
		./$$test || true; \
		echo ""; \
	done
	@echo "✓ Diagnostic tests completed"

# Kissing spheres tests
test_lattice_visualization: test_lattice_visualization.c
	@echo "Building test_lattice_visualization..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_lattice_visualization built"

test_lattice_integration: test_lattice_integration.c
	@echo "Building test_lattice_integration..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_lattice_integration built"

run-kissing-spheres: $(KISSING_SPHERES_TESTS)
	@echo ""
	@echo "════════════════════════════════════════════════════════════"
	@echo "Running Kissing Spheres Tests"
	@echo "════════════════════════════════════════════════════════════"
	@for test in $(KISSING_SPHERES_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All kissing spheres tests completed"

# ============================================================================
# PHASE 2 TESTS (File I/O and Training Functions)
# ============================================================================

test_file_io: test_file_io.c
	@echo "Building test_file_io..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_file_io built"

test_training_functions: test_training_functions.c
	@echo "Building test_training_functions..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_training_functions built"

run-phase2: $(PHASE2_TESTS)
	@echo ""
	@echo "════════════════════════════════════════════════════════════"
	@echo "Running Phase 2 Tests (File I/O & Training Functions)"
	@echo "════════════════════════════════════════════════════════════"
	@for test in $(PHASE2_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All Phase 2 tests completed"


# ============================================================================
# PHASE 3 TESTS (CrystallineAbacus Integration)
# ============================================================================

test_abacus_matrix: test_abacus_matrix.c
	@echo "Building test_abacus_matrix..."
	$(CC) $(CFLAGS) -I../math/include -o $@ $< -L.. -L../algorithms -L../math/lib -lcllm -lalgorithms -lcrystallinemath -lm -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ test_abacus_matrix built"

run-phase3: $(PHASE3_TESTS)
	@echo ""
	@echo "══════════════════════════════════════════════════════════"
	@echo "Running Phase 3 Tests (CrystallineAbacus Integration)"
	@echo "══════════════════════════════════════════════════════════"
	@for test in $(PHASE3_TESTS); do \
		echo ""; \
		LD_LIBRARY_PATH=../math/lib:$$LD_LIBRARY_PATH ./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All Phase 3 tests completed"

# ============================================================================
# 88D TRAINING TESTS
# ============================================================================

test_gradient_accumulation: test_gradient_accumulation.c
	@echo "Building test_gradient_accumulation..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_gradient_accumulation built"

test_training_loop_88d: test_training_loop_88d.c
	@echo "Building test_training_loop_88d..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_training_loop_88d built"

run-training-88d: $(TRAINING_88D_TESTS)
	@echo ""
	@echo "══════════════════════════════════════════════════════════"
	@echo "Running 88D Training Tests"
	@echo "══════════════════════════════════════════════════════════"
	@for test in $(TRAINING_88D_TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All 88D training tests completed"

# ============================================================================
# GEOMETRIC MATRIX TESTS
# ============================================================================

test_geometric_matrix_simple: test_geometric_matrix_simple.c
	@echo "Building test_geometric_matrix_simple..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_geometric_matrix_simple built"

test_geometric_integration: test_geometric_integration.c
	@echo "Building test_geometric_integration..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_geometric_integration built"

run-geometric: $(GEOMETRIC_TESTS)
	@echo ""
	@echo "════════════════════════════════════════════════════════════"
	@echo "Running Geometric Matrix Tests"
	@echo "════════════════════════════════════════════════════════════"
	@for test in $(GEOMETRIC_TESTS); do \
		echo ""; \
		LD_LIBRARY_PATH=../math/lib:$$LD_LIBRARY_PATH ./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All geometric matrix tests completed"
