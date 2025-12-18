#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 5

pthread_barrier_t barrier;

void* worker_thread(void* arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        printf("Thread %d: Starting iteration %d\n", id, i);
        fflush(stdout);
        
        // Simulate some work
        usleep(100000 * (id + 1));
        
        printf("Thread %d: Waiting at barrier (iteration %d)\n", id, i);
        fflush(stdout);
        
        // Wait at barrier
        pthread_barrier_wait(&barrier);
        
        printf("Thread %d: Passed barrier (iteration %d)\n", id, i);
        fflush(stdout);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    // Initialize barrier for NUM_THREADS + 1 (workers + main)
    pthread_barrier_init(&barrier, NULL, NUM_THREADS + 1);
    
    printf("Starting barrier test with %d threads\n", NUM_THREADS);
    
    // Create worker threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    // Main thread participates in barrier
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        printf("Main: Waiting at barrier (iteration %d)\n", i);
        fflush(stdout);
        
        pthread_barrier_wait(&barrier);
        
        printf("Main: All threads synchronized (iteration %d)\n", i);
        fflush(stdout);
    }
    
    // Wait for threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    
    printf("\n✓ Barrier test completed successfully!\n");
    return 0;
}