#ifndef RECOVERY_COMMON_H
#define RECOVERY_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Universal Recovery Toolkit - Common Library
 * 
 * This library provides common functionality for all 8 recovery tools:
 * 1. geometric-recovery
 * 2. signal-recovery
 * 3. image-recovery
 * 4. network-recovery
 * 5. crypto-recovery
 * 6. scientific-recovery
 * 7. ml-recovery
 * 8. platonic-demo
 * 
 * Key Principle: TESTS simulate failures, TOOLS recover from them.
 */

// ============================================================================
// DATA TYPES
// ============================================================================

typedef enum {
    DATA_TYPE_GEOMETRIC,
    DATA_TYPE_SIGNAL,
    DATA_TYPE_IMAGE,
    DATA_TYPE_NETWORK,
    DATA_TYPE_CRYPTO,
    DATA_TYPE_SCIENTIFIC,
    DATA_TYPE_ML,
    DATA_TYPE_UNKNOWN
} DataType;

typedef enum {
    FORMAT_OBJ,
    FORMAT_STL,
    FORMAT_PLY,
    FORMAT_WAV,
    FORMAT_FLAC,
    FORMAT_MP3,
    FORMAT_CSV,
    FORMAT_JSON,
    FORMAT_JPG,
    FORMAT_PNG,
    FORMAT_TIFF,
    FORMAT_DICOM,
    FORMAT_GML,
    FORMAT_GRAPHML,
    FORMAT_PDB,
    FORMAT_MOL2,
    FORMAT_CIF,
    FORMAT_PT,
    FORMAT_H5,
    FORMAT_NPY,
    FORMAT_ONNX,
    FORMAT_UNKNOWN
} FileFormat;

typedef enum {
    CORRUPTION_NONE,
    CORRUPTION_MISSING_DATA,      // Missing vertices, samples, pixels, nodes
    CORRUPTION_NOISE,              // Random noise added
    CORRUPTION_DISTORTION,         // Systematic distortion
    CORRUPTION_STRUCTURAL,         // Topology/connectivity errors
    CORRUPTION_MULTIPLE            // Multiple corruption types
} CorruptionType;

typedef enum {
    RECOVERY_PHASE1,  // Oscillation Analysis
    RECOVERY_PHASE2,  // Symmetry Detection
    RECOVERY_PHASE3,  // Topology Reconstruction
    RECOVERY_PHASE4,  // Recursive Stabilization
    RECOVERY_PHASE5,  // Dynamic Model Expansion
    RECOVERY_PHASE6,  // Hyper-Dimensional Analysis
    RECOVERY_AUTO     // Automatic selection
} RecoveryAlgorithm;

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

// ============================================================================
// GENERIC DATA STRUCTURES
// ============================================================================

// Generic geometric data
typedef struct {
    double* vertices;      // Vertex coordinates (x, y, z)
    size_t num_vertices;
    int* edges;           // Edge indices (pairs of vertex indices)
    size_t num_edges;
    int* faces;           // Face indices (variable length)
    size_t num_faces;
    void* metadata;       // Format-specific metadata
} GeometricData;

// Generic signal data
typedef struct {
    double* samples;      // Signal samples
    size_t num_samples;
    size_t num_channels;
    double sample_rate;
    void* metadata;       // Format-specific metadata
} SignalData;

// Generic image data
typedef struct {
    uint8_t* pixels;      // Pixel data
    size_t width;
    size_t height;
    size_t channels;      // 1=grayscale, 3=RGB, 4=RGBA
    void* metadata;       // Format-specific metadata
} ImageData;

// Generic network data
typedef struct {
    int* nodes;           // Node IDs
    size_t num_nodes;
    int* edges;           // Edge pairs (node1, node2)
    size_t num_edges;
    double* properties;   // Node/edge properties
    void* metadata;       // Format-specific metadata
} NetworkData;

// Generic cryptographic data
typedef struct {
    uint8_t** hashes;     // Array of hash values
    size_t num_hashes;
    size_t hash_size;
    void* structure;      // Chain/tree structure
    void* metadata;       // Format-specific metadata
} CryptoData;

// Generic scientific data
typedef struct {
    double* atoms;        // Atom positions (x, y, z)
    size_t num_atoms;
    int* bonds;          // Bond pairs (atom1, atom2)
    size_t num_bonds;
    double* states;      // Quantum states or other properties
    void* metadata;       // Format-specific metadata
} ScientificData;

// Generic ML data
typedef struct {
    double* weights;      // Weight values
    size_t num_weights;
    int* layers;         // Layer structure
    size_t num_layers;
    void* architecture;  // Model architecture
    void* metadata;       // Format-specific metadata
} MLData;

// Union of all data types
typedef union {
    GeometricData geometric;
    SignalData signal;
    ImageData image;
    NetworkData network;
    CryptoData crypto;
    ScientificData scientific;
    MLData ml;
} GenericData;

// ============================================================================
// CORRUPTION DETECTION
// ============================================================================

typedef struct {
    CorruptionType type;
    double severity;           // 0.0 to 1.0
    size_t num_corrupted;      // Number of corrupted elements
    size_t total_elements;     // Total number of elements
    char description[256];     // Human-readable description
} CorruptionReport;

// Detect corruption in data
CorruptionReport* detect_data_corruption(DataType type, GenericData* data);

// Free corruption report
void free_corruption_report(CorruptionReport* report);

// ============================================================================
// RECOVERY ENGINE
// ============================================================================

typedef struct {
    RecoveryAlgorithm algorithm;
    double quality;            // Recovery quality (0.0 to 1.0)
    double rmse;              // Root mean square error
    double max_error;         // Maximum error
    size_t iterations;        // Number of iterations
    double time_seconds;      // Recovery time
    bool success;             // Recovery succeeded
    char message[256];        // Status message
} RecoveryResult;

// Recover corrupted data
RecoveryResult* recover_data(DataType type, GenericData* corrupted, 
                            GenericData* recovered, RecoveryAlgorithm algorithm);

// Free recovery result
void free_recovery_result(RecoveryResult* result);

// ============================================================================
// VALIDATION
// ============================================================================

typedef struct {
    bool valid;
    double quality_score;      // 0.0 to 1.0
    double* metrics;          // Type-specific metrics
    size_t num_metrics;
    char report[1024];        // Validation report
} ValidationResult;

// Validate geometric data (Euler's formula, symmetry, manifold)
ValidationResult* validate_geometric(GeometricData* data);

// Validate signal data (SNR, THD, PESQ, STOI)
ValidationResult* validate_signal(SignalData* data, SignalData* reference);

// Validate image data (PSNR, SSIM, MS-SSIM)
ValidationResult* validate_image(ImageData* data, ImageData* reference);

// Validate network data (connectivity, centrality, clustering)
ValidationResult* validate_network(NetworkData* data);

// Validate cryptographic data (integrity, chain validation)
ValidationResult* validate_crypto(CryptoData* data);

// Validate scientific data (geometry, energy, conservation)
ValidationResult* validate_scientific(ScientificData* data);

// Validate ML data (accuracy, loss, gradients)
ValidationResult* validate_ml(MLData* data, void* test_data);

// Free validation result
void free_validation_result(ValidationResult* result);

// ============================================================================
// FILE I/O
// ============================================================================

// Detect file format from filename or content
FileFormat detect_format(const char* filename);

// Load data from file
GenericData* load_data(const char* filename, DataType* type, FileFormat* format);

// Save data to file
bool save_data(const char* filename, DataType type, GenericData* data, FileFormat format);

// Free generic data
void free_generic_data(DataType type, GenericData* data);

// ============================================================================
// METRICS
// ============================================================================

// Calculate RMSE between two datasets
double calculate_rmse(double* data1, double* data2, size_t size);

// Calculate maximum error
double calculate_max_error(double* data1, double* data2, size_t size);

// Calculate recovery rate (percentage of successfully recovered elements)
double calculate_recovery_rate(size_t recovered, size_t total);

// Calculate quality score (0.0 to 1.0)
double calculate_quality_score(double rmse, double max_error, double recovery_rate);

// ============================================================================
// VISUALIZATION
// ============================================================================

// ASCII art rendering for geometric structures
void visualize_geometric_ascii(GeometricData* data, char* output, size_t output_size);

// Plot signal waveform (ASCII)
void visualize_signal_ascii(SignalData* data, char* output, size_t output_size);

// Display image as ASCII art
void visualize_image_ascii(ImageData* data, char* output, size_t output_size);

// Display network graph (ASCII)
void visualize_network_ascii(NetworkData* data, char* output, size_t output_size);

// ============================================================================
// LOGGING
// ============================================================================

// Initialize logging
void init_logging(const char* log_file, LogLevel level);

// Log message
void log_message(LogLevel level, const char* format, ...);

// Close logging
void close_logging();

// ============================================================================
// ERROR HANDLING
// ============================================================================

typedef enum {
    ERROR_NONE = 0,
    ERROR_FILE_NOT_FOUND,
    ERROR_INVALID_FORMAT,
    ERROR_CORRUPT_DATA,
    ERROR_RECOVERY_FAILED,
    ERROR_VALIDATION_FAILED,
    ERROR_OUT_OF_MEMORY,
    ERROR_INVALID_PARAMETER,
    ERROR_UNKNOWN
} ErrorCode;

// Get last error code
ErrorCode get_last_error();

// Get error message
const char* get_error_message(ErrorCode code);

// Set error
void set_error(ErrorCode code, const char* message);

// Clear error
void clear_error();

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Get data type name
const char* get_data_type_name(DataType type);

// Get format name
const char* get_format_name(FileFormat format);

// Get corruption type name
const char* get_corruption_type_name(CorruptionType type);

// Get algorithm name
const char* get_algorithm_name(RecoveryAlgorithm algorithm);

#endif // RECOVERY_COMMON_H
