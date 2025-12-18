#include "ai/cllm_training_loop.h"
#include "ai/cllm_control_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf(" PASSED\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf(" FAILED: %s\n", msg); \
        return; \
    } while(0)

// Callback tracking
static int epoch_start_count = 0;
static int epoch_end_count = 0;
static int batch_start_count = 0;
static int batch_end_count = 0;

static void test_callback(CallbackType type, void* user_data, void* callback_data) {
    (void)user_data;
    (void)callback_data;
    
    switch (type) {
        case CALLBACK_EPOCH_START:
            epoch_start_count++;
            break;
        case CALLBACK_EPOCH_END:
            epoch_end_count++;
            break;
        case CALLBACK_BATCH_START:
            batch_start_count++;
            break;
        case CALLBACK_BATCH_END:
            batch_end_count++;
            break;
        default:
            break;
    }
}

// ============================================================================
// TEST: Training Loop Creation
// ============================================================================

void test_training_loop_create(void) {
    TEST("training_loop_create");
    
    // Create control process
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    // Create training loop
    TrainingConfiguration train_config = {
        .num_epochs = 10,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = true,
        .accumulation_steps = 4,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = true,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = true,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Verify initial state
    if (loop->state != TRAINING_STATE_IDLE) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Initial state should be IDLE");
    }
    
    // Verify configuration
    if (loop->config.num_epochs != 10) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Configuration not copied correctly");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Training Loop State Transitions
// ============================================================================

void test_training_loop_states(void) {
    TEST("training_loop_states");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Check initial state
    if (training_loop_get_state(loop) != TRAINING_STATE_IDLE) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Should start in IDLE state");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Gradient Accumulation
// ============================================================================

void test_gradient_accumulation(void) {
    TEST("gradient_accumulation");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = true,
        .accumulation_steps = 4,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Test gradient accumulation
    double gradients[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
    
    if (!training_loop_accumulate_gradients(loop, gradients, 10)) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Failed to accumulate gradients");
    }
    
    // Check accumulation count
    if (loop->gradient_buffer.accumulation_count != 1) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Accumulation count should be 1");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Gradient Clipping
// ============================================================================

void test_gradient_clipping(void) {
    TEST("gradient_clipping");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 0.5,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Set up gradients with large values
    double gradients[5] = {2.0, -3.0, 0.3, 1.5, -0.8};
    training_loop_accumulate_gradients(loop, gradients, 5);
    
    // Clip gradients
    pthread_mutex_lock(&loop->gradient_buffer.mutex);
    training_loop_clip_gradients(loop);
    
    // Check clipping
    for (size_t i = 0; i < loop->gradient_buffer.gradient_count; i++) {
        if (loop->gradient_buffer.gradients[i] > 0.5 || 
            loop->gradient_buffer.gradients[i] < -0.5) {
            pthread_mutex_unlock(&loop->gradient_buffer.mutex);
            training_loop_free(loop);
            control_process_free(cp);
            FAIL("Gradients not clipped correctly");
        }
    }
    pthread_mutex_unlock(&loop->gradient_buffer.mutex);
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Gradient Validation
// ============================================================================

void test_gradient_validation(void) {
    TEST("gradient_validation");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Test with valid gradients
    double valid_gradients[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
    training_loop_accumulate_gradients(loop, valid_gradients, 5);
    
    pthread_mutex_lock(&loop->gradient_buffer.mutex);
    bool valid = training_loop_validate_gradients(loop);
    pthread_mutex_unlock(&loop->gradient_buffer.mutex);
    
    if (!valid) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Valid gradients should pass validation");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Weight Updates
// ============================================================================

void test_weight_updates(void) {
    TEST("weight_updates");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.1,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Set up gradients
    double gradients[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
    training_loop_accumulate_gradients(loop, gradients, 5);
    
    // Update weights
    if (!training_loop_update_weights(loop)) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Failed to update weights");
    }
    
    // Check that weights were allocated
    if (!loop->weight_buffer.weights) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Weights should be allocated");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Weight Versioning
// ============================================================================

void test_weight_versioning(void) {
    TEST("weight_versioning");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Initial version should be 0
    uint32_t v1 = training_loop_get_weight_version(loop);
    if (v1 != 0) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Initial weight version should be 0");
    }
    
    // Broadcast weights
    training_loop_broadcast_weights(loop);
    
    // Version should increment
    uint32_t v2 = training_loop_get_weight_version(loop);
    if (v2 != 1) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Weight version should increment");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Callback System
// ============================================================================

void test_callback_system(void) {
    TEST("callback_system");
    
    // Reset callback counters
    epoch_start_count = 0;
    epoch_end_count = 0;
    batch_start_count = 0;
    batch_end_count = 0;
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Register callbacks
    training_loop_register_callback(loop, CALLBACK_EPOCH_START, test_callback, NULL);
    training_loop_register_callback(loop, CALLBACK_EPOCH_END, test_callback, NULL);
    training_loop_register_callback(loop, CALLBACK_BATCH_START, test_callback, NULL);
    training_loop_register_callback(loop, CALLBACK_BATCH_END, test_callback, NULL);
    
    // Trigger callbacks
    training_loop_trigger_callbacks(loop, CALLBACK_EPOCH_START, NULL);
    training_loop_trigger_callbacks(loop, CALLBACK_BATCH_START, NULL);
    training_loop_trigger_callbacks(loop, CALLBACK_BATCH_END, NULL);
    training_loop_trigger_callbacks(loop, CALLBACK_EPOCH_END, NULL);
    
    // Check callback counts
    if (epoch_start_count != 1 || epoch_end_count != 1 ||
        batch_start_count != 1 || batch_end_count != 1) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Callbacks not triggered correctly");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Learning Rate
// ============================================================================

void test_learning_rate(void) {
    TEST("learning_rate");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Check initial learning rate
    double lr1 = training_loop_get_learning_rate(loop);
    if (lr1 != 0.001) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Initial learning rate incorrect");
    }
    
    // Set new learning rate
    training_loop_set_learning_rate(loop, 0.0005);
    
    // Check updated learning rate
    double lr2 = training_loop_get_learning_rate(loop);
    if (lr2 != 0.0005) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Learning rate not updated");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Metrics
// ============================================================================

void test_metrics(void) {
    TEST("metrics");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Get metrics
    TrainingMetrics metrics;
    if (!training_loop_get_metrics(loop, &metrics)) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Failed to get metrics");
    }
    
    // Check initial metrics
    if (metrics.current_epoch != 0 || metrics.current_batch != 0) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Initial metrics incorrect");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: State String Conversion
// ============================================================================

void test_state_strings(void) {
    TEST("state_strings");
    
    const char* idle = training_loop_state_to_string(TRAINING_STATE_IDLE);
    if (strcmp(idle, "IDLE") != 0) {
        FAIL("IDLE state string incorrect");
    }
    
    const char* running = training_loop_state_to_string(TRAINING_STATE_RUNNING);
    if (strcmp(running, "RUNNING") != 0) {
        FAIL("RUNNING state string incorrect");
    }
    
    const char* paused = training_loop_state_to_string(TRAINING_STATE_PAUSED);
    if (strcmp(paused, "PAUSED") != 0) {
        FAIL("PAUSED state string incorrect");
    }
    
    PASS();
}

// ============================================================================
// TEST: Validation
// ============================================================================

void test_validation(void) {
    TEST("validation");
    
    SystemConfiguration sys_config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&sys_config);
    if (!cp) FAIL("Failed to create control process");
    
    TrainingConfiguration train_config = {
        .num_epochs = 1,
        .batch_size = 32,
        .learning_rate = 0.001,
        .learning_rate_decay = 0.95,
        .warmup_steps = 100,
        .gradient_clip_value = 1.0,
        .gradient_accumulation = false,
        .accumulation_steps = 1,
        .sync_frequency = 10,
        .async_gradient_sync = false,
        .checkpoint_frequency = 5,
        .auto_checkpoint = false,
        .max_checkpoints = 3,
        .validation_frequency = 100,
        .validation_split = 0.1,
        .profile_performance = false,
        .log_frequency = 10
    };
    strcpy(train_config.checkpoint_dir, "./test_checkpoints");
    
    TrainingLoop* loop = training_loop_create(&train_config, cp);
    if (!loop) {
        control_process_free(cp);
        FAIL("Failed to create training loop");
    }
    
    // Validate training loop
    if (!training_loop_validate(loop)) {
        training_loop_free(loop);
        control_process_free(cp);
        FAIL("Training loop should be valid");
    }
    
    training_loop_free(loop);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Training Loop Tests\n");
    printf("========================================\n\n");
    
    // Run all tests
    test_training_loop_create();
    test_training_loop_states();
    test_gradient_accumulation();
    test_gradient_clipping();
    test_gradient_validation();
    test_weight_updates();
    test_weight_versioning();
    test_callback_system();
    test_learning_rate();
    test_metrics();
    test_state_strings();
    test_validation();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("========================================\n\n");
    
    return (tests_passed == tests_run) ? 0 : 1;
}