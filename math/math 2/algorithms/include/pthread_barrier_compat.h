/**
 * @file pthread_barrier_compat.h
 * @brief pthread_barrier compatibility for macOS
 * 
 * macOS doesn't have pthread_barrier in standard pthread.h
 * This provides a compatibility implementation using condition variables
 */

#ifndef PTHREAD_BARRIER_COMPAT_H
#define PTHREAD_BARRIER_COMPAT_H

#include <pthread.h>
#include <errno.h>

#ifdef __APPLE__
// macOS doesn't have pthread_barrier - implement using condition variables

#define PTHREAD_BARRIER_SERIAL_THREAD 1

// Define barrier attribute type (unused on macOS)
typedef void pthread_barrierattr_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
} pthread_barrier_t;

static inline int pthread_barrier_init(pthread_barrier_t *barrier, 
                                       const pthread_barrierattr_t *attr, 
                                       unsigned int count) {
    (void)attr; // Unused on macOS
    
    if (count == 0) {
        return EINVAL;
    }
    
    if (pthread_mutex_init(&barrier->mutex, NULL) != 0) {
        return -1;
    }
    
    if (pthread_cond_init(&barrier->cond, NULL) != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return -1;
    }
    
    barrier->count = 0;
    barrier->trip_count = count;
    
    return 0;
}

static inline int pthread_barrier_destroy(pthread_barrier_t *barrier) {
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    return 0;
}

static inline int pthread_barrier_wait(pthread_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    
    barrier->count++;
    
    if (barrier->count >= barrier->trip_count) {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    } else {
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
        pthread_mutex_unlock(&barrier->mutex);
        return 0;
    }
}

#else
// Linux and other systems have pthread_barrier - use standard implementation
// Just ensure feature test macros are set
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#endif // __APPLE__

#endif // PTHREAD_BARRIER_COMPAT_H

