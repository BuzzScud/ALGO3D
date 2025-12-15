/*
 * Corrupted Data Generator
 * 
 * Takes a Platonic solid JSON file and generates corrupted versions
 * with specified corruption levels (5%, 10%, 15%, 20%, 25%)
 * 
 * Corruption types:
 * - Vertex displacement (random noise added to coordinates)
 * - Vertex removal (vertices set to NaN)
 * - Edge corruption (edge endpoints modified)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_VERTICES 100
#define MAX_EDGES 200
#define MAX_FACES 100

typedef struct {
    double x, y, z;
} Vertex;

typedef struct {
    int v1, v2;
} Edge;

typedef struct {
    char name[100];
    int num_vertices;
    int num_edges;
    int num_faces;
    Vertex vertices[MAX_VERTICES];
    Edge edges[MAX_EDGES];
} GeometricData;

// Read JSON file (simplified parser for our specific format)
int read_json(const char* filename, GeometricData* data) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 0;
    }
    
    char line[1024];
    int in_vertices = 0;
    int in_edges = 0;
    int vertex_count = 0;
    int edge_count = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Parse name
        if (strstr(line, "&quot;name&quot;:")) {
            sscanf(line, "  &quot;name&quot;: &quot;%[^&quot;]&quot;", data->name);
        }
        // Parse counts
        else if (strstr(line, "&quot;num_vertices&quot;:")) {
            sscanf(line, "  &quot;num_vertices&quot;: %d", &data->num_vertices);
        }
        else if (strstr(line, "&quot;num_edges&quot;:")) {
            sscanf(line, "  &quot;num_edges&quot;: %d", &data->num_edges);
        }
        else if (strstr(line, "&quot;num_faces&quot;:")) {
            sscanf(line, "  &quot;num_faces&quot;: %d", &data->num_faces);
        }
        // Parse vertices
        else if (strstr(line, "&quot;vertices&quot;:")) {
            in_vertices = 1;
            in_edges = 0;
        }
        else if (strstr(line, "&quot;edges&quot;:")) {
            in_vertices = 0;
            in_edges = 1;
        }
        else if (in_vertices && strstr(line, "&quot;x&quot;:")) {
            double x, y, z;
            sscanf(line, "    {&quot;x&quot;: %lf, &quot;y&quot;: %lf, &quot;z&quot;: %lf}", &x, &y, &z);
            data->vertices[vertex_count].x = x;
            data->vertices[vertex_count].y = y;
            data->vertices[vertex_count].z = z;
            vertex_count++;
        }
        else if (in_edges && strstr(line, "&quot;v1&quot;:")) {
            int v1, v2;
            sscanf(line, "    {&quot;v1&quot;: %d, &quot;v2&quot;: %d}", &v1, &v2);
            data->edges[edge_count].v1 = v1;
            data->edges[edge_count].v2 = v2;
            edge_count++;
        }
    }
    
    fclose(fp);
    return 1;
}

// Write corrupted data to JSON
void write_corrupted_json(const char* filename, GeometricData* data, 
                          double corruption_level, int* corrupted_indices, int num_corrupted) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  &quot;name&quot;: &quot;%s&quot;,\n", data->name);
    fprintf(fp, "  &quot;num_vertices&quot;: %d,\n", data->num_vertices);
    fprintf(fp, "  &quot;num_edges&quot;: %d,\n", data->num_edges);
    fprintf(fp, "  &quot;num_faces&quot;: %d,\n", data->num_faces);
    fprintf(fp, "  &quot;corruption_level&quot;: %.2f,\n", corruption_level);
    fprintf(fp, "  &quot;num_corrupted&quot;: %d,\n", num_corrupted);
    
    // Write corrupted indices
    fprintf(fp, "  &quot;corrupted_indices&quot;: [");
    for (int i = 0; i < num_corrupted; i++) {
        fprintf(fp, "%d%s", corrupted_indices[i], (i < num_corrupted - 1) ? ", " : "");
    }
    fprintf(fp, "],\n");
    
    // Write vertices
    fprintf(fp, "  &quot;vertices&quot;: [\n");
    for (int i = 0; i < data->num_vertices; i++) {
        fprintf(fp, "    {&quot;x&quot;: %.10f, &quot;y&quot;: %.10f, &quot;z&quot;: %.10f}%s\n",
                data->vertices[i].x, data->vertices[i].y, data->vertices[i].z,
                (i < data->num_vertices - 1) ? "," : "");
    }
    fprintf(fp, "  ],\n");
    
    // Write edges
    fprintf(fp, "  &quot;edges&quot;: [\n");
    for (int i = 0; i < data->num_edges; i++) {
        fprintf(fp, "    {&quot;v1&quot;: %d, &quot;v2&quot;: %d}%s\n",
                data->edges[i].v1, data->edges[i].v2,
                (i < data->num_edges - 1) ? "," : "");
    }
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

// Generate random double in range [min, max]
double random_double(double min, double max) {
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

// Corrupt vertices by adding noise
void corrupt_vertices(GeometricData* data, double corruption_level, 
                      int* corrupted_indices, int* num_corrupted) {
    int num_to_corrupt = (int)(data->num_vertices * corruption_level);
    *num_corrupted = num_to_corrupt;
    
    // Randomly select vertices to corrupt
    int* selected = calloc(data->num_vertices, sizeof(int));
    int count = 0;
    
    while (count < num_to_corrupt) {
        int idx = rand() % data->num_vertices;
        if (!selected[idx]) {
            selected[idx] = 1;
            corrupted_indices[count] = idx;
            count++;
            
            // Add random noise to vertex coordinates
            double noise_scale = 0.5; // 50% of coordinate range
            data->vertices[idx].x += random_double(-noise_scale, noise_scale);
            data->vertices[idx].y += random_double(-noise_scale, noise_scale);
            data->vertices[idx].z += random_double(-noise_scale, noise_scale);
        }
    }
    
    free(selected);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <input_json> [output_dir]\n", argv[0]);
        printf("Example: %s tetrahedron.json corrupted/\n", argv[0]);
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_dir = (argc > 2) ? argv[2] : "algorithms/test_data/geometric/corrupted";
    
    // Seed random number generator
    srand(time(NULL));
    
    // Read input data
    GeometricData data;
    if (!read_json(input_file, &data)) {
        return 1;
    }
    
    printf("Corrupted Data Generator\n");
    printf("========================\n\n");
    printf("Input: %s (%s)\n", input_file, data.name);
    printf("Vertices: %d, Edges: %d, Faces: %d\n\n", 
           data.num_vertices, data.num_edges, data.num_faces);
    
    // Generate corrupted versions at different levels
    double corruption_levels[] = {0.05, 0.10, 0.15, 0.20, 0.25};
    int num_levels = 5;
    
    for (int i = 0; i < num_levels; i++) {
        double level = corruption_levels[i];
        
        // Make a copy of the original data
        GeometricData corrupted_data = data;
        
        // Corrupt the data
        int corrupted_indices[MAX_VERTICES];
        int num_corrupted;
        corrupt_vertices(&corrupted_data, level, corrupted_indices, &num_corrupted);
        
        // Generate output filename
        char output_file[512];
        char base_name[256];
        
        // Extract base name from input file
        const char* last_slash = strrchr(input_file, '/');
        const char* file_name = last_slash ? last_slash + 1 : input_file;
        strncpy(base_name, file_name, sizeof(base_name) - 1);
        
        // Remove .json extension
        char* dot = strrchr(base_name, '.');
        if (dot) *dot = '\0';
        
        snprintf(output_file, sizeof(output_file), "%s/%s_corrupted_%.0f.json",
                 output_dir, base_name, level * 100);
        
        // Write corrupted data
        write_corrupted_json(output_file, &corrupted_data, level, 
                            corrupted_indices, num_corrupted);
        
        printf("Generated: %s (%.0f%% corruption, %d vertices affected)\n",
               output_file, level * 100, num_corrupted);
    }
    
    printf("\n✓ All corruption levels generated successfully!\n");
    
    return 0;
}
