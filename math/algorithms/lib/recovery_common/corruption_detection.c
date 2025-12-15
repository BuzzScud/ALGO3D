#include "recovery_common/recovery_common.h"
#include "blind_recovery/blind_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/*
 * Corruption Detection Module
 * 
 * Detects and analyzes corruption in data using the Universal Blind Recovery
 * Algorithm from OBJECTIVE 28.
 * 
 * Uses oscillation detection and structural analysis to identify:
 * - Missing data (vertices, samples, pixels, nodes)
 * - Noise/distortion
 * - Structural errors (topology, connectivity)
 * - Corruption severity
 */

// ============================================================================
// GEOMETRIC CORRUPTION DETECTION
// ============================================================================

static CorruptionReport* detect_geometric_corruption(GeometricData* data) {
    CorruptionReport* report = (CorruptionReport*)calloc(1, sizeof(CorruptionReport));
    if (!report) {
        set_error(ERROR_OUT_OF_MEMORY, "Could not allocate corruption report");
        return NULL;
    }
    
    // Check for missing data
    bool has_missing = false;
    if (!data->vertices || data->num_vertices == 0) {
        has_missing = true;
    }
    if (!data->edges || data->num_edges == 0) {
        has_missing = true;
    }
    if (!data->faces || data->num_faces == 0) {
        has_missing = true;
    }
    
    // Check Euler's formula: V - E + F = 2 (for closed manifolds)
    int euler_characteristic = (int)data->num_vertices - (int)data->num_edges + (int)data->num_faces;
    bool euler_violated = (euler_characteristic != 2);
    
    // Analyze vertex positions for outliers
    double* vertex_distances = NULL;
    size_t num_outliers = 0;
    
    if (data->vertices && data->num_vertices > 1) {
        vertex_distances = (double*)malloc(data->num_vertices * sizeof(double));
        if (vertex_distances) {
            // Calculate distances from centroid
            double centroid[3] = {0, 0, 0};
            for (size_t i = 0; i < data->num_vertices; i++) {
                centroid[0] += data->vertices[i * 3 + 0];
                centroid[1] += data->vertices[i * 3 + 1];
                centroid[2] += data->vertices[i * 3 + 2];
            }
            centroid[0] /= data->num_vertices;
            centroid[1] /= data->num_vertices;
            centroid[2] /= data->num_vertices;
            
            // Calculate mean distance
            double mean_distance = 0;
            for (size_t i = 0; i < data->num_vertices; i++) {
                double dx = data->vertices[i * 3 + 0] - centroid[0];
                double dy = data->vertices[i * 3 + 1] - centroid[1];
                double dz = data->vertices[i * 3 + 2] - centroid[2];
                vertex_distances[i] = math_sqrt(dx*dx + dy*dy + dz*dz);
                mean_distance += vertex_distances[i];
            }
            mean_distance /= data->num_vertices;
            
            // Calculate standard deviation
            double variance = 0;
            for (size_t i = 0; i < data->num_vertices; i++) {
                double diff = vertex_distances[i] - mean_distance;
                variance += diff * diff;
            }
            double std_dev = math_sqrt(variance / data->num_vertices);
            
            // Count outliers (> 3 standard deviations)
            for (size_t i = 0; i < data->num_vertices; i++) {
                if (math_abs(vertex_distances[i] - mean_distance) > 3 * std_dev) {
                    num_outliers++;
                }
            }
            
            free(vertex_distances);
        }
    }
    
    // Determine corruption type and severity
    if (has_missing) {
        report->type = CORRUPTION_MISSING_DATA;
        report->severity = 1.0; // Maximum severity for missing data
        report->num_corrupted = data->num_vertices + data->num_edges + data->num_faces;
        report->total_elements = report->num_corrupted;
        snprintf(report->description, sizeof(report->description),
                "Missing geometric data (V=%zu, E=%zu, F=%zu)",
                data->num_vertices, data->num_edges, data->num_faces);
    } else if (euler_violated) {
        report->type = CORRUPTION_STRUCTURAL;
        report->severity = 0.8;
        report->num_corrupted = abs(euler_characteristic - 2);
        report->total_elements = data->num_vertices + data->num_edges + data->num_faces;
        snprintf(report->description, sizeof(report->description),
                "Euler's formula violated (V-E+F=%d, expected 2)",
                euler_characteristic);
    } else if (num_outliers > 0) {
        report->type = CORRUPTION_DISTORTION;
        report->severity = (double)num_outliers / data->num_vertices;
        report->num_corrupted = num_outliers;
        report->total_elements = data->num_vertices;
        snprintf(report->description, sizeof(report->description),
                "Vertex position outliers detected (%zu/%zu vertices)",
                num_outliers, data->num_vertices);
    } else {
        report->type = CORRUPTION_NONE;
        report->severity = 0.0;
        report->num_corrupted = 0;
        report->total_elements = data->num_vertices + data->num_edges + data->num_faces;
        snprintf(report->description, sizeof(report->description),
                "No corruption detected");
    }
    
    return report;
}

// ============================================================================
// SIGNAL CORRUPTION DETECTION
// ============================================================================

static CorruptionReport* detect_signal_corruption(SignalData* data) {
    CorruptionReport* report = (CorruptionReport*)calloc(1, sizeof(CorruptionReport));
    if (!report) {
        set_error(ERROR_OUT_OF_MEMORY, "Could not allocate corruption report");
        return NULL;
    }
    
    // Check for missing data
    if (!data->samples || data->num_samples == 0) {
        report->type = CORRUPTION_MISSING_DATA;
        report->severity = 1.0;
        report->num_corrupted = data->num_samples;
        report->total_elements = data->num_samples;
        snprintf(report->description, sizeof(report->description),
                "Missing signal data");
        return report;
    }
    
    // Detect dropout (zero or near-zero samples)
    size_t num_dropout = 0;
    double threshold = 1e-10;
    
    for (size_t i = 0; i < data->num_samples * data->num_channels; i++) {
        if (math_abs(data->samples[i]) < threshold) {
            num_dropout++;
        }
    }
    
    // Detect clipping (samples at or near maximum)
    size_t num_clipped = 0;
    double clip_threshold = 0.99;
    
    for (size_t i = 0; i < data->num_samples * data->num_channels; i++) {
        if (math_abs(data->samples[i]) > clip_threshold) {
            num_clipped++;
        }
    }
    
    // Calculate signal statistics
    double mean = 0;
    double variance = 0;
    size_t total_samples = data->num_samples * data->num_channels;
    
    for (size_t i = 0; i < total_samples; i++) {
        mean += data->samples[i];
    }
    mean /= total_samples;
    
    for (size_t i = 0; i < total_samples; i++) {
        double diff = data->samples[i] - mean;
        variance += diff * diff;
    }
    variance /= total_samples;
    double std_dev = math_sqrt(variance);
    
    // Determine corruption type and severity
    if (num_dropout > total_samples * 0.01) { // More than 1% dropout
        report->type = CORRUPTION_MISSING_DATA;
        report->severity = (double)num_dropout / total_samples;
        report->num_corrupted = num_dropout;
        report->total_elements = total_samples;
        snprintf(report->description, sizeof(report->description),
                "Signal dropout detected (%zu/%zu samples)",
                num_dropout, total_samples);
    } else if (num_clipped > total_samples * 0.01) { // More than 1% clipping
        report->type = CORRUPTION_DISTORTION;
        report->severity = (double)num_clipped / total_samples;
        report->num_corrupted = num_clipped;
        report->total_elements = total_samples;
        snprintf(report->description, sizeof(report->description),
                "Signal clipping detected (%zu/%zu samples)",
                num_clipped, total_samples);
    } else if (std_dev > 1.0) { // High variance suggests noise
        report->type = CORRUPTION_NOISE;
        report->severity = math_min(std_dev / 10.0, 1.0); // Normalize to 0-1
        report->num_corrupted = total_samples / 10; // Estimate
        report->total_elements = total_samples;
        snprintf(report->description, sizeof(report->description),
                "High noise level detected (std_dev=%.3f)",
                std_dev);
    } else {
        report->type = CORRUPTION_NONE;
        report->severity = 0.0;
        report->num_corrupted = 0;
        report->total_elements = total_samples;
        snprintf(report->description, sizeof(report->description),
                "No corruption detected");
    }
    
    return report;
}

// ============================================================================
// IMAGE CORRUPTION DETECTION
// ============================================================================

static CorruptionReport* detect_image_corruption(ImageData* data) {
    CorruptionReport* report = (CorruptionReport*)calloc(1, sizeof(CorruptionReport));
    if (!report) {
        set_error(ERROR_OUT_OF_MEMORY, "Could not allocate corruption report");
        return NULL;
    }
    
    // Check for missing data
    if (!data->pixels || data->width == 0 || data->height == 0) {
        report->type = CORRUPTION_MISSING_DATA;
        report->severity = 1.0;
        report->num_corrupted = data->width * data->height * data->channels;
        report->total_elements = report->num_corrupted;
        snprintf(report->description, sizeof(report->description),
                "Missing image data");
        return report;
    }
    
    size_t total_pixels = data->width * data->height * data->channels;
    
    // Detect missing pixels (black or white)
    size_t num_black = 0;
    size_t num_white = 0;
    
    for (size_t i = 0; i < total_pixels; i++) {
        if (data->pixels[i] == 0) num_black++;
        if (data->pixels[i] == 255) num_white++;
    }
    
    // Calculate image statistics
    double mean = 0;
    for (size_t i = 0; i < total_pixels; i++) {
        mean += data->pixels[i];
    }
    mean /= total_pixels;
    
    double variance = 0;
    for (size_t i = 0; i < total_pixels; i++) {
        double diff = data->pixels[i] - mean;
        variance += diff * diff;
    }
    variance /= total_pixels;
    double std_dev = math_sqrt(variance);
    
    // Determine corruption type and severity
    if (num_black > total_pixels * 0.1 || num_white > total_pixels * 0.1) {
        report->type = CORRUPTION_MISSING_DATA;
        report->severity = (double)(num_black + num_white) / total_pixels;
        report->num_corrupted = num_black + num_white;
        report->total_elements = total_pixels;
        snprintf(report->description, sizeof(report->description),
                "Missing pixels detected (black=%zu, white=%zu)",
                num_black, num_white);
    } else if (std_dev < 10.0) { // Very low variance suggests corruption
        report->type = CORRUPTION_DISTORTION;
        report->severity = 1.0 - (std_dev / 128.0);
        report->num_corrupted = total_pixels / 4; // Estimate
        report->total_elements = total_pixels;
        snprintf(report->description, sizeof(report->description),
                "Low variance detected (std_dev=%.3f)",
                std_dev);
    } else {
        report->type = CORRUPTION_NONE;
        report->severity = 0.0;
        report->num_corrupted = 0;
        report->total_elements = total_pixels;
        snprintf(report->description, sizeof(report->description),
                "No corruption detected");
    }
    
    return report;
}

// ============================================================================
// NETWORK CORRUPTION DETECTION
// ============================================================================

static CorruptionReport* detect_network_corruption(NetworkData* data) {
    CorruptionReport* report = (CorruptionReport*)calloc(1, sizeof(CorruptionReport));
    if (!report) {
        set_error(ERROR_OUT_OF_MEMORY, "Could not allocate corruption report");
        return NULL;
    }
    
    // Check for missing data
    if (!data->nodes || data->num_nodes == 0 || !data->edges || data->num_edges == 0) {
        report->type = CORRUPTION_MISSING_DATA;
        report->severity = 1.0;
        report->num_corrupted = data->num_nodes + data->num_edges;
        report->total_elements = report->num_corrupted;
        snprintf(report->description, sizeof(report->description),
                "Missing network data (nodes=%zu, edges=%zu)",
                data->num_nodes, data->num_edges);
        return report;
    }
    
    // Check for disconnected nodes
    bool* connected = (bool*)calloc(data->num_nodes, sizeof(bool));
    if (!connected) {
        free(report);
        set_error(ERROR_OUT_OF_MEMORY, "Could not allocate memory");
        return NULL;
    }
    
    for (size_t i = 0; i < data->num_edges; i++) {
        int node1 = data->edges[i * 2];
        int node2 = data->edges[i * 2 + 1];
        if (node1 >= 0 && node1 < (int)data->num_nodes) connected[node1] = true;
        if (node2 >= 0 && node2 < (int)data->num_nodes) connected[node2] = true;
    }
    
    size_t num_disconnected = 0;
    for (size_t i = 0; i < data->num_nodes; i++) {
        if (!connected[i]) num_disconnected++;
    }
    
    free(connected);
    
    // Determine corruption type and severity
    if (num_disconnected > 0) {
        report->type = CORRUPTION_STRUCTURAL;
        report->severity = (double)num_disconnected / data->num_nodes;
        report->num_corrupted = num_disconnected;
        report->total_elements = data->num_nodes;
        snprintf(report->description, sizeof(report->description),
                "Disconnected nodes detected (%zu/%zu nodes)",
                num_disconnected, data->num_nodes);
    } else {
        report->type = CORRUPTION_NONE;
        report->severity = 0.0;
        report->num_corrupted = 0;
        report->total_elements = data->num_nodes + data->num_edges;
        snprintf(report->description, sizeof(report->description),
                "No corruption detected");
    }
    
    return report;
}

// ============================================================================
// GENERIC CORRUPTION DETECTION
// ============================================================================

CorruptionReport* detect_data_corruption(DataType type, GenericData* data) {
    if (!data) {
        set_error(ERROR_INVALID_PARAMETER, "Invalid data parameter");
        return NULL;
    }
    
    CorruptionReport* report = NULL;
    
    switch (type) {
        case DATA_TYPE_GEOMETRIC:
            report = detect_geometric_corruption(&data->geometric);
            break;
            
        case DATA_TYPE_SIGNAL:
            report = detect_signal_corruption(&data->signal);
            break;
            
        case DATA_TYPE_IMAGE:
            report = detect_image_corruption(&data->image);
            break;
            
        case DATA_TYPE_NETWORK:
            report = detect_network_corruption(&data->network);
            break;
            
        case DATA_TYPE_CRYPTO:
        case DATA_TYPE_SCIENTIFIC:
        case DATA_TYPE_ML:
            // TODO: Implement corruption detection for these types
            report = (CorruptionReport*)calloc(1, sizeof(CorruptionReport));
            if (report) {
                report->type = CORRUPTION_NONE;
                report->severity = 0.0;
                snprintf(report->description, sizeof(report->description),
                        "Corruption detection not yet implemented for this type");
            }
            break;
            
        default:
            set_error(ERROR_INVALID_PARAMETER, "Unknown data type");
            return NULL;
    }
    
    if (report) {
        clear_error();
    }
    
    return report;
}

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

void free_corruption_report(CorruptionReport* report) {
    if (report) {
        free(report);
    }
}
