# Makefile for libalgorithms - Mathematical Algorithms Library

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11 -D_GNU_SOURCE
LDFLAGS = -shared
ARFLAGS = rcs
INCLUDES = -I./include -I./include/blind_recovery -I../math/include

# Source files
SOURCES = src/abacus88d/abacus88d.c \
          src/generic_model.c \
          src/dimensional_frequencies.c \
          src/geometric_space_ops.c \
          src/numerical.c \
          src/loss_functions.c \
          src/optimizers.c \
          src/backprop.c \
          src/statistics.c \
          src/threading.c \
          src/shared_memory.c \
          src/shared_memory_rainbow.c \
          src/shared_memory_enhanced.c \
          src/lock_free_queue.c \
          src/sphere_packing.c \
          src/sphere_threading.c \
          src/hierarchical_memory.c \
          src/threading_integration.c \
          src/message_passing.c \
          src/state_management.c \
          src/work_distribution.c \
          src/hierarchical_threading.c \
          src/thread_parameters.c \
          src/geometric_matrix.c \
          src/thread_parameters_geometric.c \
          src/adaptive_threading.c \
          src/memory_management.c \
          src/visualization.c \
          src/visualization_2d.c \
          src/visualization_3d.c \
          src/visualization_crystalline.c \
          src/hierarchical_primes.c \
          src/hierarchical_structures.c \
          src/batch_processing.c \
          src/hierarchical_prime_partitions.c \
          src/lattice_sphere_positions.c \
          src/angular_attention.c \
          src/cymatic_modulation.c \
          src/ntt_attention.c \
          src/ntt_attention_backward.c \
          src/lattice_embeddings.c \
          src/tensor_ops.c \
          src/qk_mapping.c \
          src/iterative_search.c \
          src/validation.c \
          src/mathematical_formulas.c \
          src/symbolic_field_theory.c \
          src/nonce_generation.c \
          src/math_wrappers.c \
          src/blind_recovery/oscillation_detection.c \
          src/blind_recovery/structural_mapping.c \
          src/blind_recovery/coprime_analysis.c \
          src/blind_recovery/corruption_detection.c \
          src/blind_recovery/anchor_selection.c \
          src/blind_recovery/triangulation.c \
          src/blind_recovery/anchor_adjustment.c \
          src/blind_recovery/confidence_scoring.c \
          src/blind_recovery/candidate_generation.c \
          src/blind_recovery/fitness_scoring.c \
          src/blind_recovery/iterative_refinement.c \
          src/blind_recovery/convergence_detection.c \
          src/blind_recovery/multi_scale_analysis.c \
          src/blind_recovery/recursive_stabilization.c \
          src/blind_recovery/stabilization_metrics.c \
          src/blind_recovery/model_expansion.c \
          src/blind_recovery/self_similar_generation.c \
          src/blind_recovery/hyperdimensional_analysis.c \
          src/blind_recovery/multi_scalar_analysis.c \
          src/blind_recovery/variance_analysis.c \
          src/blind_recovery/cross_correlation.c \
          src/blind_recovery/universal_recovery.c \
          src/blind_recovery/universal_recovery_v2.c \
          src/platonic_model/tetration.c \
          src/platonic_model/platonic_model_core.c \
          src/platonic_model/platonic_model_scaling.c \
          src/platonic_model/platonic_model_oscillations.c \
          src/platonic_model/platonic_model_recovery.c \
          src/platonic_model/platonic_model_persistence.c \
          src/geometric_recovery/tetration_attractors.c \
          src/geometric_recovery/torus_analysis.c \
          src/geometric_recovery/multi_torus_tracker.c \
          src/geometric_recovery/oscillation_decomposition.c \
          src/geometric_recovery/harmonic_folding.c \
          src/geometric_recovery/kissing_spheres.c \
          src/geometric_recovery/micro_model.c \
          src/geometric_recovery/g_triangulation.c \
          src/geometric_recovery/prime_factor_extraction.c \
          src/geometric_recovery/recursive_recovery.c \
          src/geometric_recovery/clock_lattice_integration.c \
          src/geometric_recovery/spherical_recovery.c \
          src/geometric_recovery/search_recovery.c \
          src/geometric_recovery/anchor_grid_24.c \
          src/geometric_recovery/anchor_grid.c \
          src/geometric_recovery/fractal_bounds.c \
          src/geometric_recovery/multi_scale_search.c \
          src/geometric_recovery/clock_recovery.c \
          src/geometric_recovery/convergence_detection.c \
          src/geometric_recovery/oscillation_detection.c \
          src/geometric_recovery/confidence_scoring.c \
          src/geometric_recovery/geometric_recovery_orchestrator.c
# Object files
OBJECTS = $(SOURCES:.c=.o)

# Output libraries
SHARED_LIB = libalgorithms.so
STATIC_LIB = libalgorithms.a
CRYSTALLINE_LIB = ../libcrystalline.so

.PHONY: all static clean install test test_phase1 test_phase2 benchmark_phase1

all: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS) $(MATH_LIB)
	@echo "Creating shared algorithms library: $@"
	$(CC) $(LDFLAGS) -Wl,--unresolved-symbols=ignore-all -o $@ $(OBJECTS) -L.. -L../math/lib -lcrystallinemath -lssl -lcrypto -lm -lpthread
	@echo "✓ Shared algorithms library created"

static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	@echo "Creating static algorithms library: $@"
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	@echo "✓ Static algorithms library created"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB)

install: $(SHARED_LIB) $(STATIC_LIB)
	cp $(SHARED_LIB) $(STATIC_LIB) /usr/local/lib/
	cp include/*.h /usr/local/include/
	ldconfig

# Dependencies
src/numerical.o: src/numerical.c include/numerical.h
src/loss_functions.o: src/loss_functions.c include/loss_functions.h include/numerical.h
src/optimizers.o: src/optimizers.c include/optimizers.h
src/backprop.o: src/backprop.c include/backprop.h include/numerical.h
src/statistics.o: src/statistics.c include/statistics.h
src/threading.o: src/threading.c include/threading.h
src/shared_memory.o: src/shared_memory.c include/shared_memory.h
src/lock_free_queue.o: src/lock_free_queue.c include/lock_free_queue.h
src/sphere_packing.o: src/sphere_packing.c include/sphere_packing.h
src/hierarchical_primes.o: src/hierarchical_primes.c include/hierarchical_primes.h
src/hierarchical_structures.o: src/hierarchical_structures.c include/hierarchical_structures.h
src/batch_processing.o: src/batch_processing.c include/batch_processing.h

# Test targets
test: test_phase1 test_phase2


test_phase2: tests/test_phase2.c $(SHARED_LIB)
	@echo "Building Phase 2 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase2 tests/test_phase2.c -L. -lalgorithms -L.. -lm
	@echo "Running Phase 2 tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase2

test_phase3: tests/test_phase3.c $(SHARED_LIB)
	@echo "Building Phase 3 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase3 tests/test_phase3.c -L. -lalgorithms -L.. -lm
	@echo "Running Phase 3 tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase3

test_phase4: tests/test_phase4_comprehensive.c $(SHARED_LIB)
	@echo "Building Phase 4 comprehensive tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase4_comprehensive tests/test_phase4_comprehensive.c -L. -lalgorithms -L.. -lm
	@echo "Running Phase 4 comprehensive tests..."
	LD_LIBRARY_PATH=..:. ./tests/test_phase4_comprehensive

test_phase5: tests/test_phase5.c $(SHARED_LIB)
	@echo "Building Phase 5 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase5 tests/test_phase5.c -L. -lalgorithms -L.. -lm
	@echo "Running Phase 5 tests..."
	LD_LIBRARY_PATH=.:.../tests/test_phase5

test_phase6: tests/test_phase6.c $(SHARED_LIB)
	@echo "Building Phase 6 tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase6 tests/test_phase6.c -L. -lalgorithms -L.. -lm
	@echo "Running Phase 6 tests..."
	LD_LIBRARY_PATH=.:.. ./tests/test_phase6

test_torus_phase1: tests/test_torus_recovery_phase1.c $(SHARED_LIB)
	@echo "Building Phase 1 Torus Recovery test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_torus_phase1 tests/test_torus_recovery_phase1.c -L. -lalgorithms -L.. -lssl -lcrypto -lm
	@echo "Running Phase 1 Torus Recovery test..."
	LD_LIBRARY_PATH=.:.. ./tests/test_torus_phase1


test_geometric_recovery: tests/test_geometric_recovery.c $(SHARED_LIB)
	@echo "Building Geometric Recovery test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_geometric_recovery tests/test_geometric_recovery.c -L. -lalgorithms -L.. -lssl -lcrypto -lm
	@echo "Running Geometric Recovery test..."
	LD_LIBRARY_PATH=.:.. ./tests/test_geometric_recovery


test_iterative_v2: tests/test_iterative_recovery_v2.c $(SHARED_LIB)
	@echo "Building Iterative Recovery V2 test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_iterative_recovery_v2 tests/test_iterative_recovery_v2.c -L. -lalgorithms -L.. -lssl -lcrypto -lm
	@echo "Running Iterative Recovery V2 test..."
	LD_LIBRARY_PATH=.:.. ./tests/test_iterative_recovery_v2

test_candidate_analysis: tests/test_candidate_analysis.c $(SHARED_LIB)
	@echo "Building Candidate Analysis test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_candidate_analysis tests/test_candidate_analysis.c -L. -lalgorithms -L.. -lssl -lcrypto -lm
	@echo "Running Candidate Analysis test..."
	LD_LIBRARY_PATH=.:.. ./tests/test_candidate_analysis

# Phase 1 Integration Test
test_phase1: tests/test_phase1_integration.c $(SHARED_LIB)
	@echo "Building Phase 1 Integration test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase1_integration tests/test_phase1_integration.c -L. -lalgorithms -L../math/lib -lcrystallinemath
	@echo "Running Phase 1 Integration test..."
	LD_LIBRARY_PATH=.:..:../math/lib ./tests/test_phase1_integration


# Phase 1 Integration Test
test_phase1_integration: tests/test_phase1_integration.c $(SHARED_LIB)
	@echo "Building Phase 1 Integration test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_phase1_integration tests/test_phase1_integration.c -L. -lalgorithms -L.. -lcllm -L../math/lib -lcrystallinemath
	@echo "Running Phase 1 Integration test..."
	LD_LIBRARY_PATH=.:..:../math/lib ./tests/test_phase1_integration

.PHONY: test_phase1_integration

# Phase 1 Benchmarking
benchmark_phase1: tests/benchmark_phase1.c $(SHARED_LIB)
	@echo "Building Phase 1 Benchmark..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/benchmark_phase1 tests/benchmark_phase1.c -L. -lalgorithms -L../math/lib -lcrystallinemath
	@echo "Running Phase 1 Benchmark..."
	LD_LIBRARY_PATH=.:..:../math/lib ./tests/benchmark_phase1

.PHONY: benchmark_phase1

# Abacus88D Test
test_abacus88d: tests/test_abacus88d.c $(SHARED_LIB)
	@echo "Building Abacus88D test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_abacus88d tests/test_abacus88d.c -L. -lalgorithms -L.. -lcllm -L../math/lib -lcrystallinemath -lpthread
	@echo "Running Abacus88D test..."
	LD_LIBRARY_PATH=.:..:../math/lib ./tests/test_abacus88d

.PHONY: test_abacus88d

# Geometric Space Operations Test
test_geometric_space_ops: tests/test_geometric_space_ops.c $(SHARED_LIB)
	@echo "Building Geometric Space Operations test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_geometric_space_ops tests/test_geometric_space_ops.c -L. -lalgorithms -L.. -lcllm -L../math/lib -lcrystallinemath -lpthread
	@echo "Running Geometric Space Operations test..."
	LD_LIBRARY_PATH=.:..:../math/lib ./tests/test_geometric_space_ops

.PHONY: test_geometric_space_ops


# 88D Thread Pool Test
test_88d_thread_pool: tests/test_88d_thread_pool.c $(SHARED_LIB)
	@echo "Building 88D Thread Pool test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/test_88d_thread_pool tests/test_88d_thread_pool.c -L. -lalgorithms -L.. -lcllm -L../math/lib -lcrystallinemath -lpthread
	@echo "Running 88D Thread Pool test..."
	LD_LIBRARY_PATH=.:..:../math/lib ./tests/test_88d_thread_pool

.PHONY: test_88d_thread_pool
