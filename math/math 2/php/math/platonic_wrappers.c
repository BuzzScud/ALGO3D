// Platonic solid wrapper functions for PHP
// These extract data from C structures and return as PHP arrays

#include "php.h"
#include "../../math/include/math/platonic_generator.h"
#include "../../math/include/math/polytope.h"

/* Helper to convert PlatonicSolid to PHP array */
static void platonic_to_php_array(PlatonicSolid* solid, zval *return_value) {
    if (!solid) {
        RETURN_EMPTY_ARRAY();
    }
    
    array_init(return_value);
    
    // Basic properties
    add_assoc_long(return_value, "dimension", solid->dimension);
    add_assoc_long(return_value, "num_vertices", solid->num_vertices);
    add_assoc_long(return_value, "num_edges", solid->num_edges);
    add_assoc_long(return_value, "num_faces", solid->num_faces);
    add_assoc_double(return_value, "edge_length", solid->edge_length);
    add_assoc_double(return_value, "circumradius", solid->circumradius);
    add_assoc_double(return_value, "inradius", solid->inradius);
    add_assoc_double(return_value, "volume", solid->volume);
    add_assoc_string(return_value, "symmetry_group", solid->symmetry_group);
    
    // Vertices array
    zval vertices;
    array_init(&vertices);
    for (uint64_t i = 0; i < solid->num_vertices; i++) {
        zval vertex;
        array_init(&vertex);
        for (uint32_t d = 0; d < solid->dimension; d++) {
            add_next_index_double(&vertex, solid->vertex_coords[i * solid->dimension + d]);
        }
        add_next_index_zval(&vertices, &vertex);
    }
    add_assoc_zval(return_value, "vertices", &vertices);
    
    // Edges array
    zval edges;
    array_init(&edges);
    for (uint64_t i = 0; i < solid->num_edges; i++) {
        zval edge;
        array_init(&edge);
        add_next_index_long(&edge, solid->edge_indices[i][0]);
        add_next_index_long(&edge, solid->edge_indices[i][1]);
        add_next_index_zval(&edges, &edge);
    }
    add_assoc_zval(return_value, "edges", &edges);
    
    // Faces array
    zval faces;
    array_init(&faces);
    for (uint64_t i = 0; i < solid->num_faces; i++) {
        zval face;
        array_init(&face);
        for (uint32_t j = 0; j < solid->face_sizes[i]; j++) {
            add_next_index_long(&face, solid->face_indices[i][j]);
        }
        add_next_index_zval(&faces, &face);
    }
    add_assoc_zval(return_value, "faces", &faces);
    
    platonic_free(solid);
}

/* {{{ proto array platonic_tetrahedron() */
PHP_FUNCTION(platonic_tetrahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_tetrahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_cube() */
PHP_FUNCTION(platonic_cube)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_cube();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_octahedron() */
PHP_FUNCTION(platonic_octahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_octahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_dodecahedron() */
PHP_FUNCTION(platonic_dodecahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_dodecahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_icosahedron() */
PHP_FUNCTION(platonic_icosahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_icosahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_simplex(int dimension) */
PHP_FUNCTION(platonic_simplex)
{
    zend_long dimension;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dimension)
    ZEND_PARSE_PARAMETERS_END();
    
    PlatonicSolid* solid = platonic_generate_simplex((uint32_t)dimension);
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_hypercube(int dimension) */
PHP_FUNCTION(platonic_hypercube)
{
    zend_long dimension;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dimension)
    ZEND_PARSE_PARAMETERS_END();
    
    PlatonicSolid* solid = platonic_generate_hypercube((uint32_t)dimension);
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_cross_polytope(int dimension) */
PHP_FUNCTION(platonic_cross_polytope)
{
    zend_long dimension;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dimension)
    ZEND_PARSE_PARAMETERS_END();
    
    PlatonicSolid* solid = platonic_generate_cross_polytope((uint32_t)dimension);
    platonic_to_php_array(solid, return_value);
}
/* }}} */