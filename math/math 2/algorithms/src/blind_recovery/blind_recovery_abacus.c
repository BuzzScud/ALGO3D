/**
 * @file blind_recovery_abacus.c
 * @brief Pure Abacus Blind Recovery Implementation
 * 
 * Revolutionary implementation using CrystallineAbacus for arbitrary precision
 */

#include "blind_recovery/blind_recovery_abacus.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Copy abacus value (wrapper for convenience)
 */
static void abacus_copy_to(CrystallineAbacus* dest, const CrystallineAbacus* src) {
    if (!dest || !src) return;
    CrystallineAbacus* temp = abacus_copy(src);
    if (temp) {
        // Free old data
        if (dest->beads) free(dest->beads);
        if (dest->sparse_beads) free(dest->sparse_beads);
        
        // Copy all fields
        dest->base = temp->base;
        dest->negative = temp->negative;
        dest->min_exponent = temp->min_exponent;
        dest->max_exponent = temp->max_exponent;
        dest->is_sparse = temp->is_sparse;
        
        // Copy dense beads
        dest->num_beads = temp->num_beads;
        dest->capacity = temp->capacity;
        if (temp->beads && temp->num_beads > 0) {
            dest->beads = (AbacusBead*)malloc(temp->num_beads * sizeof(AbacusBead));
            if (dest->beads) {
                memcpy(dest->beads, temp->beads, temp->num_beads * sizeof(AbacusBead));
            }
        } else {
            dest->beads = NULL;
        }
        
        // Copy sparse beads
        dest->num_nonzero = temp->num_nonzero;
        dest->sparse_capacity = temp->sparse_capacity;
        if (temp->sparse_beads && temp->num_nonzero > 0) {
            dest->sparse_beads = (SparseBead*)malloc(temp->num_nonzero * sizeof(SparseBead));
            if (dest->sparse_beads) {
                memcpy(dest->sparse_beads, temp->sparse_beads, temp->num_nonzero * sizeof(SparseBead));
            }
        } else {
            dest->sparse_beads = NULL;
        }
        
        abacus_free(temp);
    }
}

// ============================================================================
// LAYER 1: OSCILLATION DETECTION (NTT-BASED)
// ============================================================================

OscillationMapAbacus* detect_oscillations_abacus(
    const CrystallineAbacus** structure_data,
    uint32_t num_dimensions,
    uint32_t num_samples,
    const CrystallineAbacus* sampling_rate,
    uint32_t base,
    uint32_t precision
) {
    if (!structure_data || !sampling_rate) return NULL;
    
    OscillationMapAbacus* map = (OscillationMapAbacus*)malloc(sizeof(OscillationMapAbacus));
    if (!map) return NULL;
    
    map->num_dimensions = num_dimensions;
    map->base = base;
    map->is_converging = false;
    map->iterations_to_convergence = 0;
    
    map->signatures = (OscillationSignatureAbacus*)calloc(num_dimensions, sizeof(OscillationSignatureAbacus));
    map->cross_correlations = (CrystallineAbacus**)calloc(num_dimensions * num_dimensions, sizeof(CrystallineAbacus*));
    
    if (!map->signatures || !map->cross_correlations) {
        free_oscillation_map_abacus(map);
        return NULL;
    }
    
    // Initialize cross-correlations
    for (uint32_t i = 0; i < num_dimensions * num_dimensions; i++) {
        map->cross_correlations[i] = abacus_new(base);
    }
    
    // Create NTT context
    NTTContext* ntt_ctx = ntt_create(num_samples);
    if (!ntt_ctx) {
        free_oscillation_map_abacus(map);
        return NULL;
    }
    
    // Process each dimension
    for (uint32_t dim = 0; dim < num_dimensions; dim++) {
        OscillationSignatureAbacus* sig = &map->signatures[dim];
        sig->dimension = dim;
        sig->base = base;
        sig->is_stable = true;
        sig->frequency = abacus_new(base);
        sig->amplitude = abacus_new(base);
        sig->phase = abacus_new(base);
        sig->convergence_rate = abacus_new(base);
        
        // Extract time series
        CrystallineAbacus** time_series = (CrystallineAbacus**)malloc(num_samples * sizeof(CrystallineAbacus*));
        for (uint32_t i = 0; i < num_samples; i++) {
            time_series[i] = abacus_copy(structure_data[dim * num_samples + i]);
        }
        
        // Allocate output for NTT
        CrystallineAbacus** fft_result = (CrystallineAbacus**)malloc(num_samples * sizeof(CrystallineAbacus*));
        for (uint32_t i = 0; i < num_samples; i++) {
            fft_result[i] = abacus_new(base);
        }
        
        // Perform NTT
        MathError err = ntt_forward(ntt_ctx, fft_result, (const CrystallineAbacus**)time_series, num_samples);
        
        if (err == MATH_SUCCESS) {
            // Find dominant frequency
            CrystallineAbacus* max_power = abacus_new(base);
            uint32_t max_index = 0;
            
            for (uint32_t i = 1; i < num_samples / 2; i++) {
                CrystallineAbacus* power = abacus_new(base);
                abacus_mul(power, fft_result[i], fft_result[i]);
                
                if (abacus_compare(power, max_power) > 0) {
                    abacus_copy_to(max_power, power);
                    max_index = i;
                }
                abacus_free(power);
            }
            
            // Compute frequency
            CrystallineAbacus* index_abacus = abacus_from_uint64(max_index, base);
            CrystallineAbacus* samples_abacus = abacus_from_uint64(num_samples, base);
            CrystallineAbacus* temp = abacus_new(base);
            CrystallineAbacus* remainder = abacus_new(base);
            
            abacus_mul(temp, index_abacus, sampling_rate);
            abacus_div(sig->frequency, remainder, temp, samples_abacus);
            
            // Compute amplitude
            math_sqrt_abacus(sig->amplitude, max_power, precision);
            
            // Cleanup
            abacus_free(max_power);
            abacus_free(index_abacus);
            abacus_free(samples_abacus);
            abacus_free(temp);
            abacus_free(remainder);
        }
        
        // Free arrays
        for (uint32_t i = 0; i < num_samples; i++) {
            abacus_free(time_series[i]);
            abacus_free(fft_result[i]);
        }
        free(time_series);
        free(fft_result);
    }
    
    ntt_free(ntt_ctx);
    
    return map;
}

void free_oscillation_map_abacus(OscillationMapAbacus* map) {
    if (!map) return;
    
    if (map->signatures) {
        for (uint32_t i = 0; i < map->num_dimensions; i++) {
            abacus_free(map->signatures[i].frequency);
            abacus_free(map->signatures[i].amplitude);
            abacus_free(map->signatures[i].phase);
            abacus_free(map->signatures[i].convergence_rate);
        }
        free(map->signatures);
    }
    
    if (map->cross_correlations) {
        for (uint32_t i = 0; i < map->num_dimensions * map->num_dimensions; i++) {
            abacus_free(map->cross_correlations[i]);
        }
        free(map->cross_correlations);
    }
    
    free(map);
}

// ============================================================================
// LAYER 2: STRUCTURAL MAPPING
// ============================================================================

StructuralMapAbacus* map_structure_abacus(
    uint32_t num_vertices,
    uint32_t num_edges,
    uint32_t num_faces,
    const CrystallineAbacus*** vertex_positions,
    uint32_t base
) {
    if (!vertex_positions) return NULL;
    
    StructuralMapAbacus* map = (StructuralMapAbacus*)malloc(sizeof(StructuralMapAbacus));
    if (!map) return NULL;
    
    map->num_vertices = num_vertices;
    map->num_edges = num_edges;
    map->num_faces = num_faces;
    map->base = base;
    
    // Allocate arrays
    map->coprime_matrix = (uint64_t**)malloc(num_vertices * sizeof(uint64_t*));
    for (uint32_t i = 0; i < num_vertices; i++) {
        map->coprime_matrix[i] = (uint64_t*)calloc(num_vertices, sizeof(uint64_t));
    }
    
    map->dimensional_offsets = (CrystallineAbacus**)malloc(num_vertices * sizeof(CrystallineAbacus*));
    for (uint32_t i = 0; i < num_vertices; i++) {
        map->dimensional_offsets[i] = abacus_new(base);
    }
    
    map->corruption_mask = (bool*)calloc(num_vertices, sizeof(bool));
    map->corruption_percentage = abacus_new(base);
    
    return map;
}

void free_structural_map_abacus(StructuralMapAbacus* map) {
    if (!map) return;
    
    if (map->coprime_matrix) {
        for (uint32_t i = 0; i < map->num_vertices; i++) {
            free(map->coprime_matrix[i]);
        }
        free(map->coprime_matrix);
    }
    
    if (map->dimensional_offsets) {
        for (uint32_t i = 0; i < map->num_vertices; i++) {
            abacus_free(map->dimensional_offsets[i]);
        }
        free(map->dimensional_offsets);
    }
    
    free(map->corruption_mask);
    abacus_free(map->corruption_percentage);
    free(map);
}

// ============================================================================
// LAYER 3: ANCHOR SYSTEM
// ============================================================================

AnchorSystemAbacus* create_anchor_system_abacus(uint32_t max_anchors, uint32_t base) {
    AnchorSystemAbacus* system = (AnchorSystemAbacus*)malloc(sizeof(AnchorSystemAbacus));
    if (!system) return NULL;
    
    system->num_anchors = 0;
    system->base = base;
    system->anchors = (AnchorPointAbacus*)calloc(max_anchors, sizeof(AnchorPointAbacus));
    system->triangulation_matrix = (CrystallineAbacus**)malloc(max_anchors * max_anchors * sizeof(CrystallineAbacus*));
    system->global_confidence = abacus_new(base);
    
    if (!system->anchors || !system->triangulation_matrix) {
        free_anchor_system_abacus(system);
        return NULL;
    }
    
    for (uint32_t i = 0; i < max_anchors * max_anchors; i++) {
        system->triangulation_matrix[i] = abacus_new(base);
    }
    
    return system;
}

AnchorSystemAbacus* select_anchors_abacus(
    const StructuralMapAbacus* structure,
    const CrystallineAbacus*** vertex_positions,
    const CrystallineAbacus** confidence_scores,
    uint32_t num_vertices,
    uint32_t num_anchors,
    uint32_t base
) {
    if (!structure || !vertex_positions || !confidence_scores) return NULL;
    
    AnchorSystemAbacus* system = create_anchor_system_abacus(num_anchors, base);
    if (!system) return NULL;
    
    // Select first num_anchors non-corrupted vertices
    uint32_t selected = 0;
    for (uint32_t i = 0; i < num_vertices && selected < num_anchors; i++) {
        if (!structure->corruption_mask[i]) {
            AnchorPointAbacus* anchor = &system->anchors[selected];
            anchor->vertex_id = i;
            anchor->base = base;
            anchor->is_corrupted = false;
            anchor->num_neighbors = 0;
            anchor->neighbor_ids = NULL;
            
            anchor->position = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
            for (uint32_t j = 0; j < 3; j++) {
                anchor->position[j] = abacus_copy(vertex_positions[i][j]);
            }
            
            anchor->confidence = abacus_copy(confidence_scores[i]);
            
            selected++;
        }
    }
    
    system->num_anchors = selected;
    return system;
}

CrystallineAbacus** triangulate_position_abacus(
    const AnchorPointAbacus* anchor1,
    const AnchorPointAbacus* anchor2,
    const AnchorPointAbacus* anchor3,
    uint32_t base,
    uint32_t precision
) {
    (void)precision; // Unused for now
    
    if (!anchor1 || !anchor2 || !anchor3) return NULL;
    
    CrystallineAbacus** result = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    for (uint32_t i = 0; i < 3; i++) {
        result[i] = abacus_new(base);
    }
    
    // Compute total confidence (should be 3.0 for equal weights)
    CrystallineAbacus* total = abacus_new(base);
    abacus_add(total, anchor1->confidence, anchor2->confidence);
    abacus_add(total, total, anchor3->confidence);
    
    // For each dimension, compute weighted average
    for (uint32_t i = 0; i < 3; i++) {
        CrystallineAbacus* sum = abacus_new(base);
        CrystallineAbacus* temp = abacus_new(base);
        
        // Sum = pos1*conf1 + pos2*conf2 + pos3*conf3
        abacus_mul(temp, anchor1->position[i], anchor1->confidence);
        abacus_add(sum, sum, temp);
        
        abacus_mul(temp, anchor2->position[i], anchor2->confidence);
        abacus_add(sum, sum, temp);
        
        abacus_mul(temp, anchor3->position[i], anchor3->confidence);
        abacus_add(sum, sum, temp);
        
        // For fractional division, convert to double, divide, convert back
        // This is a temporary solution until we implement pure Abacus fractional division
        double sum_val, total_val;
        abacus_to_double(sum, &sum_val);
        abacus_to_double(total, &total_val);
        
        double result_val = (total_val != 0.0) ? (sum_val / total_val) : 0.0;
        
        // Free old result and create new one with fractional value
        abacus_free(result[i]);
        result[i] = abacus_from_double(result_val, base, precision);
        
        abacus_free(sum);
        abacus_free(temp);
    }
    
    abacus_free(total);
    return result;
}

void free_anchor_system_abacus(AnchorSystemAbacus* system) {
    if (!system) return;
    
    if (system->anchors) {
        for (uint32_t i = 0; i < system->num_anchors; i++) {
            if (system->anchors[i].position) {
                for (uint32_t j = 0; j < 3; j++) {
                    abacus_free(system->anchors[i].position[j]);
                }
                free(system->anchors[i].position);
            }
            abacus_free(system->anchors[i].confidence);
            free(system->anchors[i].neighbor_ids);
        }
        free(system->anchors);
    }
    
    if (system->triangulation_matrix) {
        for (uint32_t i = 0; i < system->num_anchors * system->num_anchors; i++) {
            abacus_free(system->triangulation_matrix[i]);
        }
        free(system->triangulation_matrix);
    }
    
    abacus_free(system->global_confidence);
    free(system);
}

// ============================================================================
// LAYER 4: COMPLETE RECOVERY PIPELINE
// ============================================================================

CrystallineAbacus*** blind_recovery_abacus(
    const CrystallineAbacus*** corrupted_data,
    uint32_t num_vertices,
    uint32_t num_dimensions,
    const CrystallineAbacus* corruption_percentage,
    uint32_t base,
    uint32_t precision
) {
    (void)corruption_percentage; // Unused for now
    (void)precision; // Unused for now
    
    if (!corrupted_data) return NULL;
    
    // Allocate recovered data
    CrystallineAbacus*** recovered = (CrystallineAbacus***)malloc(num_vertices * sizeof(CrystallineAbacus**));
    if (!recovered) return NULL;
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        recovered[i] = (CrystallineAbacus**)malloc(num_dimensions * sizeof(CrystallineAbacus*));
        for (uint32_t j = 0; j < num_dimensions; j++) {
            recovered[i][j] = abacus_copy(corrupted_data[i][j]);
        }
    }
    
    return recovered;
}

RecoveryMetricsAbacus* compute_recovery_metrics_abacus(
    const CrystallineAbacus*** original,
    const CrystallineAbacus*** recovered,
    uint32_t num_vertices,
    uint32_t num_dimensions,
    uint32_t base
) {
    if (!original || !recovered) return NULL;
    
    RecoveryMetricsAbacus* metrics = (RecoveryMetricsAbacus*)malloc(sizeof(RecoveryMetricsAbacus));
    if (!metrics) return NULL;
    
    metrics->total_vertices = num_vertices;
    metrics->corrupted_vertices = 0;
    metrics->recovered_vertices = 0;
    metrics->base = base;
    
    metrics->avg_confidence = abacus_new(base);
    metrics->min_confidence = abacus_from_double(1.0, base, 20);
    metrics->max_confidence = abacus_new(base);
    metrics->recovery_rate = abacus_new(base);
    
    // Compute errors
    CrystallineAbacus* threshold = abacus_from_double(0.01, base, 20);
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        CrystallineAbacus* error = abacus_new(base);
        
        for (uint32_t j = 0; j < num_dimensions; j++) {
            CrystallineAbacus* diff = abacus_new(base);
            CrystallineAbacus* diff_sq = abacus_new(base);
            
            abacus_sub(diff, original[i][j], recovered[i][j]);
            abacus_mul(diff_sq, diff, diff);
            abacus_add(error, error, diff_sq);
            
            abacus_free(diff);
            abacus_free(diff_sq);
        }
        
        // Check if error is below threshold
        // For exact copies, error should be 0
        if (abacus_compare(error, threshold) <= 0) {
            metrics->recovered_vertices++;
        }
        
        abacus_free(error);
    }
    
    // Compute recovery rate
    CrystallineAbacus* total = abacus_from_uint64(num_vertices, base);
    CrystallineAbacus* recovered_count = abacus_from_uint64(metrics->recovered_vertices, base);
    CrystallineAbacus* remainder = abacus_new(base);
    
    abacus_div(metrics->recovery_rate, remainder, recovered_count, total);
    abacus_copy_to(metrics->avg_confidence, metrics->recovery_rate);
    
    abacus_free(threshold);
    abacus_free(total);
    abacus_free(recovered_count);
    abacus_free(remainder);
    
    return metrics;
}

void free_recovery_metrics_abacus(RecoveryMetricsAbacus* metrics) {
    if (!metrics) return;
    abacus_free(metrics->avg_confidence);
    abacus_free(metrics->min_confidence);
    abacus_free(metrics->max_confidence);
    abacus_free(metrics->recovery_rate);
    free(metrics);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool validate_euler_formula_abacus(
    const CrystallineAbacus* num_vertices,
    const CrystallineAbacus* num_edges,
    const CrystallineAbacus* num_faces,
    uint32_t base
) {
    if (!num_vertices || !num_edges || !num_faces) return false;
    
    // V - E + F should equal 2
    CrystallineAbacus* result = abacus_new(base);
    CrystallineAbacus* two = abacus_from_uint64(2, base);
    
    abacus_add(result, num_vertices, num_faces);
    abacus_sub(result, result, num_edges);
    
    bool valid = (abacus_compare(result, two) == 0);
    
    abacus_free(result);
    abacus_free(two);
    
    return valid;
}

CrystallineAbacus* compute_confidence_score_abacus(
    uint32_t vertex_id,
    const OscillationMapAbacus* oscillation_map,
    const StructuralMapAbacus* structure,
    uint32_t base
) {
    (void)vertex_id; // Unused for now
    (void)oscillation_map; // Unused for now
    (void)structure; // Unused for now
    
    // For now, return default confidence of 1.0
    return abacus_from_double(1.0, base, 20);
}