/**
 * multi_torus_tracker.c - Multi-Torus Tracking and Intersection
 */

#include "../include/multi_torus_tracker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

MultiTorusTracker* create_multi_torus_tracker(
    int max_tori,
    int history_size,
    uint64_t original_space
) {
    MultiTorusTracker* tracker = (MultiTorusTracker*)calloc(1, sizeof(MultiTorusTracker));
    if (!tracker) return NULL;
    
    tracker->tori = (TorusDescriptor*)calloc(max_tori, sizeof(TorusDescriptor));
    tracker->k_history = (double*)calloc(history_size, sizeof(double));
    
    if (!tracker->tori || !tracker->k_history) {
        free_multi_torus_tracker(tracker);
        return NULL;
    }
    
    tracker->max_tori = max_tori;
    tracker->history_size = history_size;
    tracker->history_index = 0;
    tracker->num_tori = 0;
    tracker->original_space = original_space;
    tracker->reduced_space = original_space;
    tracker->reduction_factor = 1.0;
    tracker->intersection_valid = false;
    
    return tracker;
}

void free_multi_torus_tracker(MultiTorusTracker* tracker) {
    if (!tracker) return;
    if (tracker->tori) free(tracker->tori);
    if (tracker->k_history) free(tracker->k_history);
    free(tracker);
}

void multi_torus_add_sample(
    MultiTorusTracker* tracker,
    double k_estimate
) {
    if (!tracker) return;
    
    tracker->k_history[tracker->history_index] = k_estimate;
    tracker->history_index = (tracker->history_index + 1) % tracker->history_size;
}

int identify_tori(MultiTorusTracker* tracker) {
    if (!tracker) return 0;
    
    // Create oscillation decomposer
    OscillationDecomposer* decomposer = create_oscillation_decomposer(
        tracker->k_history,
        tracker->history_size,
        tracker->max_tori
    );
    
    if (!decomposer) return 0;
    
    // Decompose oscillations
    int num_components = decompose_oscillations(decomposer);
    
    // Create torus descriptor for each component
    tracker->num_tori = num_components;
    
    for (int i = 0; i < num_components; i++) {
        const OscillationComponent* comp = get_oscillation_component(decomposer, i);
        if (!comp) continue;
        
        TorusDescriptor* torus = &tracker->tori[i];
        
        torus->torus_id = i + 1;
        torus->frequency = comp->frequency;
        torus->amplitude = comp->amplitude;
        torus->phase = comp->phase;
        torus->period = comp->period;
        
        // Compute torus geometry
        torus->major_radius = comp->amplitude;
        torus->minor_radius = comp->amplitude * 0.5;  // Simplified estimate
        
        // Compute center k from mean of signal
        double sum = 0.0;
        for (int j = 0; j < tracker->history_size; j++) {
            sum += tracker->k_history[j];
        }
        torus->center_k = sum / tracker->history_size;
        
        // Compute bounds
        // Use tighter bounds (0.5× amplitude instead of full amplitude)
        // This reduces the bound size while still capturing the oscillation
        double bound_multiplier = 0.5;
        torus->k_min = torus->center_k - (torus->amplitude * bound_multiplier);
        torus->k_max = torus->center_k + (torus->amplitude * bound_multiplier);
        
        // Clip to valid k range [0, original_space]
        if (torus->k_min < 0.0) {
            torus->k_min = 0.0;
        }
        if (torus->k_max > (double)tracker->original_space) {
            torus->k_max = (double)tracker->original_space;
        }
        
        // Confidence based on amplitude relative to signal
        double signal_energy = 0.0;
        for (int j = 0; j < tracker->history_size; j++) {
            signal_energy += tracker->k_history[j] * tracker->k_history[j];
        }
        double component_energy = comp->amplitude * comp->amplitude * tracker->history_size / 2.0;
        torus->confidence = component_energy / signal_energy;
        
        torus->samples_used = tracker->history_size;
    }
    
    free_oscillation_decomposer(decomposer);
    
    return tracker->num_tori;
}

bool compute_torus_intersection(MultiTorusTracker* tracker) {
    if (!tracker || tracker->num_tori == 0) return false;
    
    // Initialize intersection bounds with first torus
    tracker->intersection_k_min = tracker->tori[0].k_min;
    tracker->intersection_k_max = tracker->tori[0].k_max;
    
    // Intersect with all other tori
    for (int i = 1; i < tracker->num_tori; i++) {
        TorusDescriptor* torus = &tracker->tori[i];
        
        // Update intersection bounds
        if (torus->k_min > tracker->intersection_k_min) {
            tracker->intersection_k_min = torus->k_min;
        }
        if (torus->k_max < tracker->intersection_k_max) {
            tracker->intersection_k_max = torus->k_max;
        }
    }
    
    // Check if intersection is valid (non-empty)
    if (tracker->intersection_k_min <= tracker->intersection_k_max) {
        tracker->intersection_valid = true;
        
        // Compute reduced space size
        double intersection_size = tracker->intersection_k_max - tracker->intersection_k_min;
        tracker->reduced_space = (uint64_t)intersection_size;
        
        // Compute reduction factor
        if (tracker->reduced_space > 0) {
            tracker->reduction_factor = (double)tracker->original_space / (double)tracker->reduced_space;
        }
        
        return true;
    } else {
        tracker->intersection_valid = false;
        return false;
    }
}

const TorusDescriptor* get_torus(
    const MultiTorusTracker* tracker,
    int index
) {
    if (!tracker || index < 0 || index >= tracker->num_tori) {
        return NULL;
    }
    return &tracker->tori[index];
}

bool is_k_in_intersection(
    const MultiTorusTracker* tracker,
    double k
) {
    if (!tracker || !tracker->intersection_valid) return false;
    
    return (k >= tracker->intersection_k_min && k <= tracker->intersection_k_max);
}

double get_multi_torus_reduction_factor(const MultiTorusTracker* tracker) {
    return tracker ? tracker->reduction_factor : 1.0;
}

void print_multi_torus_analysis(const MultiTorusTracker* tracker) {
    if (!tracker) return;
    
    printf("\n=== Multi-Torus Analysis ===\n\n");
    printf("Number of tori identified: %d\n", tracker->num_tori);
    printf("Original search space: %lu\n", tracker->original_space);
    printf("\n");
    
    for (int i = 0; i < tracker->num_tori; i++) {
        const TorusDescriptor* torus = &tracker->tori[i];
        printf("Torus %d:\n", torus->torus_id);
        printf("  Frequency: %.6f Hz\n", torus->frequency);
        printf("  Period: %d iterations\n", torus->period);
        printf("  Amplitude: %.6f\n", torus->amplitude);
        printf("  Phase: %.6f radians\n", torus->phase);
        printf("  Major radius (R): %.6f\n", torus->major_radius);
        printf("  Minor radius (r): %.6f\n", torus->minor_radius);
        printf("  Center k: %.6f\n", torus->center_k);
        printf("  k range: [%.6f, %.6f]\n", torus->k_min, torus->k_max);
        printf("  Confidence: %.4f\n", torus->confidence);
        printf("\n");
    }
    
    if (tracker->intersection_valid) {
        printf("Intersection:\n");
        printf("  Valid: YES\n");
        printf("  k range: [%.6f, %.6f]\n", 
               tracker->intersection_k_min, tracker->intersection_k_max);
        printf("  Reduced space: %lu\n", tracker->reduced_space);
        printf("  Reduction factor: %.2fx\n", tracker->reduction_factor);
        printf("\n");
    } else {
        printf("Intersection:\n");
        printf("  Valid: NO (tori do not overlap)\n");
        printf("\n");
    }
}

void export_multi_torus_csv(
    const MultiTorusTracker* tracker,
    const char* filename
) {
    if (!tracker || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "torus_id,frequency,period,amplitude,phase,major_radius,minor_radius,center_k,k_min,k_max,confidence\n");
    
    for (int i = 0; i < tracker->num_tori; i++) {
        const TorusDescriptor* torus = &tracker->tori[i];
        fprintf(f, "%d,%.6f,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.4f\n",
                torus->torus_id,
                torus->frequency,
                torus->period,
                torus->amplitude,
                torus->phase,
                torus->major_radius,
                torus->minor_radius,
                torus->center_k,
                torus->k_min,
                torus->k_max,
                torus->confidence);
    }
    
    fclose(f);
}
/**
 * Compute intersection of multiple samples
 */
MultiSampleIntersection* compute_multi_sample_intersection(
    MultiTorusTracker** trackers,
    uint32_t num_samples,
    uint64_t true_k,
    uint64_t max_k
) {
    if (!trackers || num_samples == 0) return NULL;
    
    MultiSampleIntersection* result = malloc(sizeof(MultiSampleIntersection));
    if (!result) return NULL;
    
    result->num_samples = num_samples;
    result->sample_ids = malloc(num_samples * sizeof(uint32_t));
    result->original_space = (double)max_k;
    
    // Initialize with first sample's intersection
    result->intersection_k_min = trackers[0]->intersection_k_min;
    result->intersection_k_max = trackers[0]->intersection_k_max;
    result->sample_ids[0] = 0;
    
    // Compute intersection with all other samples
    for (uint32_t i = 1; i < num_samples; i++) {
        result->sample_ids[i] = i;
        
        // Intersection is the overlap of all bounds
        if (trackers[i]->intersection_k_min > result->intersection_k_min) {
            result->intersection_k_min = trackers[i]->intersection_k_min;
        }
        if (trackers[i]->intersection_k_max < result->intersection_k_max) {
            result->intersection_k_max = trackers[i]->intersection_k_max;
        }
    }
    
    // Check if intersection is valid
    if (result->intersection_k_max <= result->intersection_k_min) {
        // No intersection - use union instead
        result->intersection_k_min = trackers[0]->intersection_k_min;
        result->intersection_k_max = trackers[0]->intersection_k_max;
        
        for (uint32_t i = 1; i < num_samples; i++) {
            if (trackers[i]->intersection_k_min < result->intersection_k_min) {
                result->intersection_k_min = trackers[i]->intersection_k_min;
            }
            if (trackers[i]->intersection_k_max > result->intersection_k_max) {
                result->intersection_k_max = trackers[i]->intersection_k_max;
            }
        }
    }
    
    // Compute final metrics
    result->intersection_size = result->intersection_k_max - result->intersection_k_min;
    result->reduction_factor = result->original_space / result->intersection_size;
    result->contains_true_k = ((double)true_k >= result->intersection_k_min && 
                               (double)true_k <= result->intersection_k_max);
    
    return result;
}

/**
 * Free multi-sample intersection
 */
void free_multi_sample_intersection(MultiSampleIntersection* intersection) {
    if (!intersection) return;
    free(intersection->sample_ids);
    free(intersection);
}
