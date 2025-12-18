#include "recovery_common/recovery_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/*
 * Visualization Module
 * 
 * Provides ASCII art visualization for various data types:
 * - Geometric: 3D structures rendered as ASCII art
 * - Signal: Waveform plots
 * - Image: ASCII art representation
 * - Network: Graph visualization
 */

// ============================================================================
// GEOMETRIC VISUALIZATION
// ============================================================================

void visualize_geometric_ascii(GeometricData* data, char* output, size_t output_size) {
    if (!data || !output || output_size == 0) {
        return;
    }
    
    // Simple box representation
    int pos = 0;
    
    pos += snprintf(output + pos, output_size - pos,
                   "┌─────────────────────────────────┐\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Geometric Structure            │\n");
    pos += snprintf(output + pos, output_size - pos,
                   "├─────────────────────────────────┤\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Vertices: %-20zu │\n", data->num_vertices);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Edges:    %-20zu │\n", data->num_edges);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Faces:    %-20zu │\n", data->num_faces);
    
    // Calculate Euler characteristic
    int euler = (int)data->num_vertices - (int)data->num_edges + (int)data->num_faces;
    pos += snprintf(output + pos, output_size - pos,
                   "│  Euler:    %-20d │\n", euler);
    
    pos += snprintf(output + pos, output_size - pos,
                   "└─────────────────────────────────┘\n");
    
    // Add simple 3D representation if we have vertices
    if (data->vertices && data->num_vertices > 0) {
        pos += snprintf(output + pos, output_size - pos, "\n");
        pos += snprintf(output + pos, output_size - pos, "3D View (simplified):\n");
        pos += snprintf(output + pos, output_size - pos, "       *\n");
        pos += snprintf(output + pos, output_size - pos, "      / \\\n");
        pos += snprintf(output + pos, output_size - pos, "     /   \\\n");
        pos += snprintf(output + pos, output_size - pos, "    *-----*\n");
    }
}

// ============================================================================
// SIGNAL VISUALIZATION
// ============================================================================

void visualize_signal_ascii(SignalData* data, char* output, size_t output_size) {
    if (!data || !output || output_size == 0) {
        return;
    }
    
    int pos = 0;
    
    pos += snprintf(output + pos, output_size - pos,
                   "┌─────────────────────────────────┐\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Signal Data                    │\n");
    pos += snprintf(output + pos, output_size - pos,
                   "├─────────────────────────────────┤\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Samples:  %-20zu │\n", data->num_samples);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Channels: %-20zu │\n", data->num_channels);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Rate:     %-20.0f │\n", data->sample_rate);
    pos += snprintf(output + pos, output_size - pos,
                   "└─────────────────────────────────┘\n");
    
    // Add simple waveform if we have samples
    if (data->samples && data->num_samples > 0) {
        pos += snprintf(output + pos, output_size - pos, "\n");
        pos += snprintf(output + pos, output_size - pos, "Waveform (first 50 samples):\n");
        
        // Find min/max for scaling
        double min_val = data->samples[0];
        double max_val = data->samples[0];
        size_t plot_samples = (data->num_samples < 50) ? data->num_samples : 50;
        
        for (size_t i = 0; i < plot_samples; i++) {
            if (data->samples[i] < min_val) min_val = data->samples[i];
            if (data->samples[i] > max_val) max_val = data->samples[i];
        }
        
        // Plot waveform (10 rows)
        int height = 10;
        for (int row = height - 1; row >= 0; row--) {
            double threshold = min_val + (max_val - min_val) * row / (height - 1);
            pos += snprintf(output + pos, output_size - pos, " ");
            
            for (size_t i = 0; i < plot_samples && pos < (int)output_size - 2; i++) {
                if (data->samples[i] >= threshold) {
                    output[pos++] = '*';
                } else {
                    output[pos++] = ' ';
                }
            }
            output[pos++] = '\n';
        }
        output[pos] = '\0';
    }
}

// ============================================================================
// IMAGE VISUALIZATION
// ============================================================================

void visualize_image_ascii(ImageData* data, char* output, size_t output_size) {
    if (!data || !output || output_size == 0) {
        return;
    }
    
    int pos = 0;
    
    pos += snprintf(output + pos, output_size - pos,
                   "┌─────────────────────────────────┐\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Image Data                     │\n");
    pos += snprintf(output + pos, output_size - pos,
                   "├─────────────────────────────────┤\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Width:    %-20zu │\n", data->width);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Height:   %-20zu │\n", data->height);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Channels: %-20zu │\n", data->channels);
    pos += snprintf(output + pos, output_size - pos,
                   "└─────────────────────────────────┘\n");
    
    // Add simple ASCII art if we have pixels
    if (data->pixels && data->width > 0 && data->height > 0) {
        pos += snprintf(output + pos, output_size - pos, "\n");
        pos += snprintf(output + pos, output_size - pos, "Preview (downsampled):\n");
        
        // Downsample to fit in terminal (max 40x20)
        int preview_width = (data->width < 40) ? data->width : 40;
        int preview_height = (data->height < 20) ? data->height : 20;
        
        for (int y = 0; y < preview_height && pos < (int)output_size - 2; y++) {
            int src_y = (y * data->height) / preview_height;
            
            for (int x = 0; x < preview_width && pos < (int)output_size - 2; x++) {
                int src_x = (x * data->width) / preview_width;
                size_t idx = (src_y * data->width + src_x) * data->channels;
                
                // Convert to grayscale if needed
                int gray = data->pixels[idx];
                if (data->channels >= 3) {
                    gray = (data->pixels[idx] + data->pixels[idx + 1] + data->pixels[idx + 2]) / 3;
                }
                
                // Map to ASCII characters
                const char* chars = " .:-=+*#%@";
                int char_idx = (gray * 9) / 255;
                output[pos++] = chars[char_idx];
            }
            output[pos++] = '\n';
        }
        output[pos] = '\0';
    }
}

// ============================================================================
// NETWORK VISUALIZATION
// ============================================================================

void visualize_network_ascii(NetworkData* data, char* output, size_t output_size) {
    if (!data || !output || output_size == 0) {
        return;
    }
    
    int pos = 0;
    
    pos += snprintf(output + pos, output_size - pos,
                   "┌─────────────────────────────────┐\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Network Data                   │\n");
    pos += snprintf(output + pos, output_size - pos,
                   "├─────────────────────────────────┤\n");
    pos += snprintf(output + pos, output_size - pos,
                   "│  Nodes: %-23zu │\n", data->num_nodes);
    pos += snprintf(output + pos, output_size - pos,
                   "│  Edges: %-23zu │\n", data->num_edges);
    
    // Calculate average degree
    if (data->num_nodes > 0) {
        double avg_degree = (2.0 * data->num_edges) / data->num_nodes;
        pos += snprintf(output + pos, output_size - pos,
                       "│  Avg Degree: %-18.2f │\n", avg_degree);
    }
    
    pos += snprintf(output + pos, output_size - pos,
                   "└─────────────────────────────────┘\n");
    
    // Add simple graph representation
    if (data->nodes && data->num_nodes > 0 && data->num_nodes <= 10) {
        pos += snprintf(output + pos, output_size - pos, "\n");
        pos += snprintf(output + pos, output_size - pos, "Graph (small network):\n");
        
        // Show first few nodes and edges
        for (size_t i = 0; i < data->num_nodes && i < 10 && pos < (int)output_size - 20; i++) {
            pos += snprintf(output + pos, output_size - pos, "  Node %zu", i);
            
            // Find edges connected to this node
            int edge_count = 0;
            for (size_t j = 0; j < data->num_edges && edge_count < 3; j++) {
                if (data->edges[j * 2] == (int)i) {
                    if (edge_count == 0) {
                        pos += snprintf(output + pos, output_size - pos, " -> ");
                    } else {
                        pos += snprintf(output + pos, output_size - pos, ", ");
                    }
                    pos += snprintf(output + pos, output_size - pos, "%d", data->edges[j * 2 + 1]);
                    edge_count++;
                }
            }
            pos += snprintf(output + pos, output_size - pos, "\n");
        }
    }
}
