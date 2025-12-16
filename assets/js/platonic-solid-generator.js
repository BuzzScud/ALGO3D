/**
 * Platonic Solid Generator
 * Generates Platonic solids and discovers new geometric structures
 * Ported from algorithms/src/platonic_model/platonic_model_core.c
 * 
 * Used for mapping hyper-dimensional space and discovering new solids
 */

const PlatonicSolidGenerator = (function() {
    'use strict';
    
    const PHI = (1 + Math.sqrt(5)) / 2; // Golden ratio
    const PI = Math.PI;
    
    /**
     * Platonic Solid Types
     */
    const SOLID_TYPES = {
        TETRAHEDRON: 0,
        CUBE: 1,
        OCTAHEDRON: 2,
        DODECAHEDRON: 3,
        ICOSAHEDRON: 4
    };
    
    /**
     * Generate base Platonic solid vertices in 3D
     */
    function generateBaseVertices3D(solidType) {
        const vertices = [];
        let numVertices = 0;
        
        switch (solidType) {
            case SOLID_TYPES.TETRAHEDRON:
                numVertices = 4;
                vertices.push(
                    [1.0, 1.0, 1.0],
                    [1.0, -1.0, -1.0],
                    [-1.0, 1.0, -1.0],
                    [-1.0, -1.0, 1.0]
                );
                break;
                
            case SOLID_TYPES.CUBE:
                numVertices = 8;
                for (let i = 0; i < 8; i++) {
                    vertices.push([
                        (i & 1) ? 1.0 : -1.0,
                        (i & 2) ? 1.0 : -1.0,
                        (i & 4) ? 1.0 : -1.0
                    ]);
                }
                break;
                
            case SOLID_TYPES.OCTAHEDRON:
                numVertices = 6;
                vertices.push(
                    [1.0, 0.0, 0.0],
                    [-1.0, 0.0, 0.0],
                    [0.0, 1.0, 0.0],
                    [0.0, -1.0, 0.0],
                    [0.0, 0.0, 1.0],
                    [0.0, 0.0, -1.0]
                );
                break;
                
            case SOLID_TYPES.DODECAHEDRON:
                numVertices = 20;
                const a = 1.0;
                const b = 1.0 / PHI;
                const c = PHI;
                
                // 8 vertices of a cube
                for (let i = 0; i < 8; i++) {
                    vertices.push([
                        (i & 1) ? a : -a,
                        (i & 2) ? a : -a,
                        (i & 4) ? a : -a
                    ]);
                }
                
                // 12 vertices on rectangular faces
                const coords = [
                    [0, b, c], [0, -b, c], [0, b, -c], [0, -b, -c],
                    [c, 0, b], [c, 0, -b], [-c, 0, b], [-c, 0, -b],
                    [b, c, 0], [-b, c, 0], [b, -c, 0], [-b, -c, 0]
                ];
                coords.forEach(coord => vertices.push(coord));
                break;
                
            case SOLID_TYPES.ICOSAHEDRON:
                numVertices = 12;
                const a2 = 1.0;
                const b2 = PHI;
                
                const icoCoords = [
                    [0, a2, b2], [0, -a2, b2], [0, a2, -b2], [0, -a2, -b2],
                    [b2, 0, a2], [b2, 0, -a2], [-b2, 0, a2], [-b2, 0, -a2],
                    [a2, b2, 0], [-a2, b2, 0], [a2, -b2, 0], [-a2, -b2, 0]
                ];
                icoCoords.forEach(coord => vertices.push(coord));
                break;
        }
        
        return { vertices, numVertices };
    }
    
    /**
     * Expand 3D vertices to high dimensions using prime-based projection
     */
    function expandToHighDimensions(vertices3D, numDimensions) {
        const primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71];
        const verticesHD = [];
        
        for (let v = 0; v < vertices3D.length; v++) {
            const vertexHD = [];
            const vertex3D = vertices3D[v];
            
            // Copy original 3D coordinates
            vertexHD[0] = vertex3D[0];
            vertexHD[1] = vertex3D[1];
            vertexHD[2] = vertex3D[2];
            
            // Project into higher dimensions using prime-based formula
            for (let d = 3; d < numDimensions; d++) {
                let sum = 0.0;
                for (let i = 0; i < 3; i++) {
                    const coord = vertex3D[i];
                    const prime = primes[d % primes.length];
                    sum += coord * Math.cos(2.0 * PI * prime * (d - 3) / numDimensions);
                }
                vertexHD[d] = sum / Math.sqrt(numDimensions);
            }
            
            verticesHD.push(vertexHD);
        }
        
        return verticesHD;
    }
    
    /**
     * Platonic Solid Model
     */
    class PlatonicSolid {
        constructor(solidType, numDimensions = 13, numVertices = null) {
            this.solidType = solidType;
            this.numDimensions = Math.max(13, numDimensions);
            this.vertices = [];
            this.metadata = {
                name: this.getSolidName(solidType),
                baseVertices: 0,
                expandedVertices: 0,
                dimensions: this.numDimensions
            };
            
            this.generate(solidType, numVertices);
        }
        
        /**
         * Generate the solid
         */
        generate(solidType, targetVertices) {
            // Generate base 3D vertices
            const { vertices: baseVertices, numVertices } = generateBaseVertices3D(solidType);
            this.metadata.baseVertices = numVertices;
            
            // Expand to high dimensions
            this.vertices = expandToHighDimensions(baseVertices, this.numDimensions);
            this.metadata.expandedVertices = this.vertices.length;
            
            // Replicate vertices if target is higher
            if (targetVertices && targetVertices > this.vertices.length) {
                this.replicateVertices(targetVertices);
            }
        }
        
        /**
         * Replicate vertices to reach target count
         */
        replicateVertices(targetCount) {
            const baseCount = this.vertices.length;
            const replicationFactor = Math.ceil(targetCount / baseCount);
            
            for (let r = 1; r < replicationFactor; r++) {
                for (let v = 0; v < baseCount && this.vertices.length < targetCount; v++) {
                    const baseVertex = this.vertices[v];
                    const newVertex = baseVertex.map((coord, d) => {
                        // Add small perturbation based on replication index
                        const perturbation = Math.sin(r * PI * (d + 1) / this.numDimensions) * 0.1;
                        return coord + perturbation;
                    });
                    this.vertices.push(newVertex);
                }
            }
            
            this.metadata.expandedVertices = this.vertices.length;
        }
        
        /**
         * Get solid name
         */
        getSolidName(solidType) {
            const names = ['Tetrahedron', 'Cube', 'Octahedron', 'Dodecahedron', 'Icosahedron'];
            return names[solidType] || 'Unknown';
        }
        
        /**
         * Calculate geometric properties
         */
        calculateProperties() {
            if (this.vertices.length === 0) return null;
            
            // Calculate center
            const center = new Array(this.numDimensions).fill(0);
            for (let v = 0; v < this.vertices.length; v++) {
                for (let d = 0; d < this.numDimensions; d++) {
                    center[d] += this.vertices[v][d];
                }
            }
            for (let d = 0; d < this.numDimensions; d++) {
                center[d] /= this.vertices.length;
            }
            
            // Calculate average distance from center
            let totalDistance = 0;
            for (let v = 0; v < this.vertices.length; v++) {
                let distSq = 0;
                for (let d = 0; d < this.numDimensions; d++) {
                    const diff = this.vertices[v][d] - center[d];
                    distSq += diff * diff;
                }
                totalDistance += Math.sqrt(distSq);
            }
            const avgRadius = totalDistance / this.vertices.length;
            
            // Calculate volume estimate (for high dimensions)
            const volumeEstimate = Math.pow(avgRadius, this.numDimensions) * 
                                 Math.pow(PI, this.numDimensions / 2) / 
                                 this.gamma(this.numDimensions / 2 + 1);
            
            return {
                center: center,
                averageRadius: avgRadius,
                volumeEstimate: volumeEstimate,
                vertexCount: this.vertices.length
            };
        }
        
        /**
         * Gamma function approximation (for volume calculation)
         */
        gamma(z) {
            // Stirling's approximation for large z
            if (z > 10) {
                return Math.sqrt(2 * PI / z) * Math.pow((z / Math.E), z);
            }
            // Simple approximation for smaller z
            let result = 1.0;
            const n = Math.floor(z);
            for (let i = 1; i < n; i++) {
                result *= i;
            }
            // Handle fractional part
            if (z !== n) {
                result *= Math.sqrt(PI); // Approximation for fractional gamma
            }
            return result;
        }
        
        /**
         * Project to lower dimensions for visualization
         */
        projectToDimensions(targetDims) {
            if (targetDims >= this.numDimensions) {
                return this.vertices;
            }
            
            return this.vertices.map(vertex => {
                return vertex.slice(0, targetDims);
            });
        }
    }
    
    /**
     * Discover new solids using tetration attractors
     */
    function discoverNewSolid(baseSolid, tetrationTowers, numDimensions) {
        if (!baseSolid || !tetrationTowers || tetrationTowers.length === 0) {
            return null;
        }
        
        const newVertices = [];
        
        // For each vertex, find nearest tetration attractor
        for (let v = 0; v < baseSolid.vertices.length; v++) {
            const vertex = baseSolid.vertices[v];
            const attractor = TetrationTowers.findHDAttractor(vertex, tetrationTowers);
            
            if (attractor) {
                // Blend original vertex with attractor
                const blended = vertex.map((coord, d) => {
                    const attractorValue = attractor[d] || coord;
                    return coord * 0.7 + attractorValue * 0.3;
                });
                newVertices.push(blended);
            } else {
                newVertices.push(vertex);
            }
        }
        
        // Create new solid
        const newSolid = new PlatonicSolid(baseSolid.solidType, numDimensions);
        newSolid.vertices = newVertices;
        newSolid.metadata.name = `Discovered ${baseSolid.metadata.name}`;
        newSolid.metadata.discovered = true;
        newSolid.metadata.baseSolid = baseSolid.metadata.name;
        
        return newSolid;
    }
    
    /**
     * Generate all 5 Platonic solids
     */
    function generateAllSolids(numDimensions = 13) {
        const solids = [];
        
        for (let type = 0; type <= 4; type++) {
            try {
                const solid = new PlatonicSolid(type, numDimensions);
                solids.push(solid);
            } catch (e) {
                console.warn(`Failed to generate solid type ${type}:`, e);
            }
        }
        
        return solids;
    }
    
    return {
        PlatonicSolid,
        SOLID_TYPES,
        generateAllSolids,
        discoverNewSolid,
        expandToHighDimensions
    };
})();

// Make available globally
if (typeof window !== 'undefined') {
    window.PlatonicSolidGenerator = PlatonicSolidGenerator;
}


