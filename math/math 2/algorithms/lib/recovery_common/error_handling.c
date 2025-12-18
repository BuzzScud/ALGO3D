#include "recovery_common/recovery_common.h"
#include <stdio.h>
#include <string.h>

/*
 * Error Handling Module
 * 
 * Provides global error state management for the recovery toolkit.
 */

// Global error state
static ErrorCode g_last_error = ERROR_NONE;
static char g_error_message[256] = {0};

// ============================================================================
// ERROR MANAGEMENT
// ============================================================================

ErrorCode get_last_error() {
    return g_last_error;
}

const char* get_error_message(ErrorCode code) {
    switch (code) {
        case ERROR_NONE:
            return "No error";
        case ERROR_FILE_NOT_FOUND:
            return "File not found";
        case ERROR_INVALID_FORMAT:
            return "Invalid file format";
        case ERROR_CORRUPT_DATA:
            return "Data is corrupted";
        case ERROR_RECOVERY_FAILED:
            return "Recovery failed";
        case ERROR_VALIDATION_FAILED:
            return "Validation failed";
        case ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        case ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

void set_error(ErrorCode code, const char* message) {
    g_last_error = code;
    if (message) {
        strncpy(g_error_message, message, sizeof(g_error_message) - 1);
        g_error_message[sizeof(g_error_message) - 1] = '\0';
    } else {
        strncpy(g_error_message, get_error_message(code), sizeof(g_error_message) - 1);
        g_error_message[sizeof(g_error_message) - 1] = '\0';
    }
}

void clear_error() {
    g_last_error = ERROR_NONE;
    g_error_message[0] = '\0';
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* get_data_type_name(DataType type) {
    switch (type) {
        case DATA_TYPE_GEOMETRIC: return "Geometric";
        case DATA_TYPE_SIGNAL: return "Signal";
        case DATA_TYPE_IMAGE: return "Image";
        case DATA_TYPE_NETWORK: return "Network";
        case DATA_TYPE_CRYPTO: return "Cryptographic";
        case DATA_TYPE_SCIENTIFIC: return "Scientific";
        case DATA_TYPE_ML: return "Machine Learning";
        case DATA_TYPE_UNKNOWN:
        default: return "Unknown";
    }
}

const char* get_format_name(FileFormat format) {
    switch (format) {
        case FORMAT_OBJ: return "OBJ";
        case FORMAT_STL: return "STL";
        case FORMAT_PLY: return "PLY";
        case FORMAT_WAV: return "WAV";
        case FORMAT_FLAC: return "FLAC";
        case FORMAT_MP3: return "MP3";
        case FORMAT_CSV: return "CSV";
        case FORMAT_JSON: return "JSON";
        case FORMAT_JPG: return "JPEG";
        case FORMAT_PNG: return "PNG";
        case FORMAT_TIFF: return "TIFF";
        case FORMAT_DICOM: return "DICOM";
        case FORMAT_GML: return "GML";
        case FORMAT_GRAPHML: return "GraphML";
        case FORMAT_PDB: return "PDB";
        case FORMAT_MOL2: return "MOL2";
        case FORMAT_CIF: return "CIF";
        case FORMAT_PT: return "PyTorch";
        case FORMAT_H5: return "HDF5";
        case FORMAT_NPY: return "NumPy";
        case FORMAT_ONNX: return "ONNX";
        case FORMAT_UNKNOWN:
        default: return "Unknown";
    }
}

const char* get_corruption_type_name(CorruptionType type) {
    switch (type) {
        case CORRUPTION_NONE: return "None";
        case CORRUPTION_MISSING_DATA: return "Missing Data";
        case CORRUPTION_NOISE: return "Noise";
        case CORRUPTION_DISTORTION: return "Distortion";
        case CORRUPTION_STRUCTURAL: return "Structural";
        case CORRUPTION_MULTIPLE: return "Multiple";
        default: return "Unknown";
    }
}

const char* get_algorithm_name(RecoveryAlgorithm algorithm) {
    switch (algorithm) {
        case RECOVERY_PHASE1: return "Phase 1: Oscillation Analysis";
        case RECOVERY_PHASE2: return "Phase 2: Symmetry Detection";
        case RECOVERY_PHASE3: return "Phase 3: Topology Reconstruction";
        case RECOVERY_PHASE4: return "Phase 4: Recursive Stabilization";
        case RECOVERY_PHASE5: return "Phase 5: Dynamic Model Expansion";
        case RECOVERY_PHASE6: return "Phase 6: Hyper-Dimensional Analysis";
        case RECOVERY_AUTO: return "Auto-Select";
        default: return "Unknown";
    }
}
