# Test Data Format Specification

## Overview

This document specifies the JSON format for geometric test data used in the Universal Blind Recovery Algorithm testing framework.

---

## Geometric Data Format

### Platonic Solids

All Platonic solid data files follow this JSON structure:

```json
{
  "name": "SolidName",
  "num_vertices": <integer>,
  "num_edges": <integer>,
  "num_faces": <integer>,
  "euler_verified": <boolean>,
  "vertices": [
    {"x": <double>, "y": <double>, "z": <double>},
    ...
  ],
  "edges": [
    {"v1": <integer>, "v2": <integer>},
    ...
  ],
  "faces": [
    {"vertices": [<integer>, <integer>, ...]},
    ...
  ]
}
```

### Field Descriptions

#### Metadata Fields
- **name** (string): Name of the Platonic solid
  - Valid values: "Tetrahedron", "Cube", "Octahedron", "Dodecahedron", "Icosahedron"
  
- **num_vertices** (integer): Total number of vertices
  - Tetrahedron: 4
  - Cube: 8
  - Octahedron: 6
  - Dodecahedron: 20
  - Icosahedron: 12

- **num_edges** (integer): Total number of edges
  - Tetrahedron: 6
  - Cube: 12
  - Octahedron: 12
  - Dodecahedron: 30
  - Icosahedron: 30

- **num_faces** (integer): Total number of faces
  - Tetrahedron: 4
  - Cube: 6
  - Octahedron: 8
  - Dodecahedron: 12
  - Icosahedron: 20

- **euler_verified** (boolean): Whether Euler's formula (V - E + F = 2) is satisfied
  - Must always be `true` for valid Platonic solids

#### Geometric Data

- **vertices** (array): List of vertex coordinates
  - Each vertex has three coordinates: x, y, z
  - Coordinates are double-precision floating-point numbers
  - Vertices are indexed from 0 to (num_vertices - 1)
  - All solids are centered at the origin (0, 0, 0)

- **edges** (array): List of edges connecting vertices
  - Each edge connects two vertices: v1 and v2
  - Vertex indices reference the vertices array
  - Edges are undirected (v1→v2 is same as v2→v1)

- **faces** (array): List of faces
  - Each face contains a list of vertex indices
  - Vertices are listed in order around the face
  - Face types:
    - Triangular: 3 vertices (Tetrahedron, Octahedron, Icosahedron)
    - Square: 4 vertices (Cube)
    - Pentagonal: 5 vertices (Dodecahedron)

---

## Corrupted Data Format

Corrupted versions of geometric data include additional metadata:

```json
{
  "name": "SolidName",
  "num_vertices": <integer>,
  "num_edges": <integer>,
  "num_faces": <integer>,
  "euler_verified": <boolean>,
  "corruption_level": <double>,
  "num_corrupted": <integer>,
  "corrupted_indices": [<integer>, ...],
  "vertices": [...],
  "edges": [...],
  "faces": [...]
}
```

### Additional Fields for Corrupted Data

- **corruption_level** (double): Fraction of vertices corrupted
  - Range: 0.0 to 1.0
  - Standard levels: 0.05, 0.10, 0.15, 0.20, 0.25
  - Example: 0.15 means 15% of vertices are corrupted

- **num_corrupted** (integer): Actual number of vertices corrupted
  - Calculated as: floor(num_vertices × corruption_level)
  - May be 0 for small solids with low corruption levels

- **corrupted_indices** (array): Indices of corrupted vertices
  - List of integers referencing the vertices array
  - Sorted in ascending order
  - Length equals num_corrupted

### Corruption Method

Vertices are corrupted by adding random noise to their coordinates:

- **Noise Scale**: ±0.5 units (50% of coordinate range)
- **Distribution**: Uniform random distribution
- **Applied to**: x, y, and z coordinates independently
- **Selection**: Random selection without replacement

---

## File Naming Conventions

### Original Platonic Solids
```
<solid_name>.json
```

Examples:
- `tetrahedron.json`
- `cube.json`
- `octahedron.json`
- `dodecahedron.json`
- `icosahedron.json`

### Corrupted Versions
```
<solid_name>_corrupted_<percentage>.json
```

Examples:
- `cube_corrupted_5.json` (5% corruption)
- `icosahedron_corrupted_15.json` (15% corruption)
- `dodecahedron_corrupted_25.json` (25% corruption)

---

## Directory Structure

```
algorithms/test_data/geometric/
├── platonic/              # Original Platonic solids
│   ├── tetrahedron.json
│   ├── cube.json
│   ├── octahedron.json
│   ├── dodecahedron.json
│   └── icosahedron.json
├── corrupted/             # Corrupted versions
│   ├── tetrahedron_corrupted_5.json
│   ├── tetrahedron_corrupted_10.json
│   ├── tetrahedron_corrupted_15.json
│   ├── tetrahedron_corrupted_20.json
│   ├── tetrahedron_corrupted_25.json
│   ├── cube_corrupted_5.json
│   └── ... (25 files total)
├── archimedean/           # Future: Archimedean solids
├── johnson/               # Future: Johnson solids
├── geodesic/              # Future: Geodesic spheres
└── 4d_polytopes/          # Future: 4D polytopes
```

---

## Mathematical Properties

### Euler's Formula

All Platonic solids satisfy Euler's formula:

```
V - E + F = 2
```

Where:
- V = number of vertices
- E = number of edges
- F = number of faces

### Verification

| Solid | V | E | F | V-E+F | Valid |
|-------|---|---|---|-------|-------|
| Tetrahedron | 4 | 6 | 4 | 2 | ✓ |
| Cube | 8 | 12 | 6 | 2 | ✓ |
| Octahedron | 6 | 12 | 8 | 2 | ✓ |
| Dodecahedron | 20 | 30 | 12 | 2 | ✓ |
| Icosahedron | 12 | 30 | 20 | 2 | ✓ |

### Golden Ratio

The dodecahedron and icosahedron use the golden ratio (φ):

```
φ = (1 + √5) / 2 ≈ 1.618033988749895
```

This appears in their vertex coordinates.

---

## Usage Examples

### Loading Data (Python)

```python
import json

# Load original solid
with open('algorithms/test_data/geometric/platonic/cube.json', 'r') as f:
    cube = json.load(f)

print(f"Solid: {cube['name']}")
print(f"Vertices: {cube['num_vertices']}")
print(f"Euler verified: {cube['euler_verified']}")

# Access first vertex
v0 = cube['vertices'][0]
print(f"Vertex 0: ({v0['x']}, {v0['y']}, {v0['z']})")

# Load corrupted version
with open('algorithms/test_data/geometric/corrupted/cube_corrupted_15.json', 'r') as f:
    corrupted_cube = json.load(f)

print(f"Corruption level: {corrupted_cube['corruption_level']*100}%")
print(f"Corrupted vertices: {corrupted_cube['corrupted_indices']}")
```

### Loading Data (C)

```c
#include <stdio.h>
#include <json-c/json.h>

// Parse JSON file
struct json_object *root = json_object_from_file("cube.json");

// Get metadata
struct json_object *name_obj;
json_object_object_get_ex(root, "name", &name_obj);
const char *name = json_object_get_string(name_obj);

// Get vertices array
struct json_object *vertices_obj;
json_object_object_get_ex(root, "vertices", &vertices_obj);
int num_vertices = json_object_array_length(vertices_obj);

// Iterate through vertices
for (int i = 0; i < num_vertices; i++) {
    struct json_object *vertex = json_object_array_get_idx(vertices_obj, i);
    
    struct json_object *x_obj, *y_obj, *z_obj;
    json_object_object_get_ex(vertex, "x", &x_obj);
    json_object_object_get_ex(vertex, "y", &y_obj);
    json_object_object_get_ex(vertex, "z", &z_obj);
    
    double x = json_object_get_double(x_obj);
    double y = json_object_get_double(y_obj);
    double z = json_object_get_double(z_obj);
    
    printf("Vertex %d: (%.2f, %.2f, %.2f)\n", i, x, y, z);
}
```

---

## Validation

### Required Checks

1. **Euler's Formula**: V - E + F = 2
2. **Vertex Count**: Matches num_vertices
3. **Edge Count**: Matches num_edges
4. **Face Count**: Matches num_faces
5. **Edge Validity**: All edge indices < num_vertices
6. **Face Validity**: All face vertex indices < num_vertices
7. **Corruption Metadata**: For corrupted files only
   - corruption_level in range [0.0, 1.0]
   - num_corrupted ≤ num_vertices
   - All corrupted_indices < num_vertices
   - corrupted_indices sorted and unique

### Validation Tool

A validation tool is provided:

```bash
./algorithms/tools/bin/validate_geometric_data <json_file>
```

---

## Future Extensions

### Planned Additions

1. **Archimedean Solids** (13 types)
   - Truncated tetrahedron, cuboctahedron, etc.
   - Semi-regular polyhedra

2. **Johnson Solids** (92 types)
   - Convex polyhedra with regular faces
   - Not uniform

3. **Geodesic Spheres**
   - Various frequencies (1-10)
   - Icosahedral subdivision

4. **4D Polytopes**
   - Tesseract (4D hypercube)
   - 120-cell, 600-cell
   - 4D coordinate format

### Format Extensions

Future formats may include:

- **Symmetry groups**: Explicit symmetry operations
- **Dual relationships**: Links to dual polyhedra
- **Sphere packing**: Kissing sphere configurations
- **Frequency domain**: Fourier transform data
- **Prime mappings**: Clock lattice positions

---

## Version History

- **v1.0** (2024-12-08): Initial specification
  - Platonic solids format
  - Corrupted data format
  - File naming conventions

---

**Last Updated**: December 8, 2024  
**Status**: Active - v1.0  
**Maintainer**: Crystalline CLLM Project