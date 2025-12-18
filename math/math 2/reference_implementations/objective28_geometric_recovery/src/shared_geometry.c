/**
 * @file shared_geometry.c
 * @brief Find shared vertices and faces (all k share geometry)
 */

#include "geometric_recovery.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

/**
 * Find nearest vertex to a position
 */
static uint32_t find_nearest_vertex(
    const double* position,
    PlatonicModel* model,
    uint32_t num_dimensions
) {
    uint32_t nearest = 0;
    double min_dist_sq = 1e10;
    
    for (uint32_t v = 0; v < model->num_vertices; v++) {
        double dist_sq = 0.0;
        for (uint32_t d = 0; d < num_dimensions; d++) {
            double diff = position[d] - model->vertex_positions[v * num_dimensions + d];
            dist_sq += diff * diff;
        }
        
        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            nearest = v;
        }
    }
    
    return nearest;
}

/**
 * Find shared vertices (vertices with multiple k values)
 * 
 * Key insight: "All k should share a face or line up along a vertex"
 */
SharedVertex* find_shared_vertices(
    const double** k_positions,
    uint32_t num_k_values,
    PlatonicModel* model,
    uint32_t* num_shared_out
) {
    // Map each k to its nearest vertex
    uint32_t* vertex_map = calloc(num_k_values, sizeof(uint32_t));
    uint32_t* vertex_counts = calloc(model->num_vertices, sizeof(uint32_t));
    
    for (uint32_t i = 0; i < num_k_values; i++) {
        vertex_map[i] = find_nearest_vertex(k_positions[i], model, model->num_dimensions);
        vertex_counts[vertex_map[i]]++;
    }
    
    // Find vertices with multiple k values (shared vertices)
    uint32_t num_shared = 0;
    for (uint32_t v = 0; v < model->num_vertices; v++) {
        if (vertex_counts[v] > 1) {
            num_shared++;
        }
    }
    
    // Allocate shared vertices
    SharedVertex* shared = calloc(num_shared, sizeof(SharedVertex));
    uint32_t shared_idx = 0;
    
    for (uint32_t v = 0; v < model->num_vertices; v++) {
        if (vertex_counts[v] > 1) {
            SharedVertex* sv = &shared[shared_idx++];
            sv->vertex_id = v;
            sv->num_k_values = vertex_counts[v];
            
            // Copy vertex position
            for (uint32_t d = 0; d < model->num_dimensions; d++) {
                sv->position[d] = model->vertex_positions[v * model->num_dimensions + d];
            }
            
            // Collect k indices at this vertex
            sv->k_indices = calloc(sv->num_k_values, sizeof(uint32_t));
            uint32_t k_idx = 0;
            for (uint32_t i = 0; i < num_k_values; i++) {
                if (vertex_map[i] == v) {
                    sv->k_indices[k_idx++] = i;
                }
            }
            
            // Confidence based on number of k values
            sv->confidence = (double)sv->num_k_values / num_k_values;
            
            // Find faces this vertex belongs to
            // (Simplified - in full implementation, use model topology)
            sv->num_faces = 0;
            sv->face_ids = NULL;
        }
    }
    
    free(vertex_map);
    free(vertex_counts);
    
    *num_shared_out = num_shared;
    return shared;
}

/**
 * Check if vertex is in face
 */
static bool vertex_in_face(uint32_t vertex_id, const SharedFace* face) {
    for (uint32_t i = 0; i < face->num_vertices; i++) {
        if (face->vertex_ids[i] == vertex_id) {
            return true;
        }
    }
    return false;
}

/**
 * Find shared faces (faces containing shared vertices)
 * 
 * Key insight: "All k should share a face or line up along a vertex"
 */
SharedFace* find_shared_faces(
    const SharedVertex* shared_vertices,
    uint32_t num_shared_vertices,
    PlatonicModel* model,
    uint32_t* num_faces_out
) {
    if (num_shared_vertices == 0 || model->num_faces == 0) {
        *num_faces_out = 0;
        return NULL;
    }
    
    // Allocate faces (upper bound)
    SharedFace* faces = calloc(model->num_faces, sizeof(SharedFace));
    uint32_t num_faces = 0;
    
    // For each face in the model
    for (uint32_t f = 0; f < model->num_faces; f++) {
        // Check if this face contains any shared vertices
        uint32_t num_shared_in_face = 0;
        
        // Get face vertices (simplified - assumes triangular faces)
        uint32_t face_vertices[3];
        for (uint32_t i = 0; i < 3 && i < model->num_vertices; i++) {
            face_vertices[i] = (f * 3 + i) % model->num_vertices;
        }
        
        // Count shared vertices in this face
        for (uint32_t sv = 0; sv < num_shared_vertices; sv++) {
            for (uint32_t fv = 0; fv < 3; fv++) {
                if (shared_vertices[sv].vertex_id == face_vertices[fv]) {
                    num_shared_in_face++;
                    break;
                }
            }
        }
        
        // If face has shared vertices, add it
        if (num_shared_in_face > 0) {
            SharedFace* sf = &faces[num_faces++];
            sf->face_id = f;
            sf->num_vertices = 3;
            sf->vertex_ids = calloc(3, sizeof(uint32_t));
            memcpy(sf->vertex_ids, face_vertices, 3 * sizeof(uint32_t));
            
            // Collect k indices on this face
            sf->num_k_values = 0;
            sf->k_indices = calloc(num_shared_vertices * 10, sizeof(uint32_t));  // Upper bound
            
            for (uint32_t sv = 0; sv < num_shared_vertices; sv++) {
                if (vertex_in_face(shared_vertices[sv].vertex_id, sf)) {
                    for (uint32_t k = 0; k < shared_vertices[sv].num_k_values; k++) {
                        sf->k_indices[sf->num_k_values++] = shared_vertices[sv].k_indices[k];
                    }
                }
            }
            
            // Compute face normal (simplified for 3D)
            memset(sf->normal, 0, sizeof(sf->normal));
            if (model->num_dimensions >= 3) {
                // Cross product of two edges
                double edge1[3], edge2[3];
                for (uint32_t d = 0; d < 3; d++) {
                    edge1[d] = model->vertex_positions[face_vertices[1] * model->num_dimensions + d] -
                              model->vertex_positions[face_vertices[0] * model->num_dimensions + d];
                    edge2[d] = model->vertex_positions[face_vertices[2] * model->num_dimensions + d] -
                              model->vertex_positions[face_vertices[0] * model->num_dimensions + d];
                }
                
                sf->normal[0] = edge1[1] * edge2[2] - edge1[2] * edge2[1];
                sf->normal[1] = edge1[2] * edge2[0] - edge1[0] * edge2[2];
                sf->normal[2] = edge1[0] * edge2[1] - edge1[1] * edge2[0];
                
                // Normalize
                double norm = math_sqrt(sf->normal[0]*sf->normal[0] + 
                                        sf->normal[1]*sf->normal[1] + 
                                        sf->normal[2]*sf->normal[2]);
                if (norm > 1e-6) {
                    sf->normal[0] /= norm;
                    sf->normal[1] /= norm;
                    sf->normal[2] /= norm;
                }
            }
            
            // Confidence based on number of k values
            sf->confidence = (double)sf->num_k_values / (num_shared_vertices * 10);
        }
    }
    
    *num_faces_out = num_faces;
    return faces;
}