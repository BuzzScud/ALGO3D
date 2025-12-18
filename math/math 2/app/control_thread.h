// control_thread.h - Control Thread Interface
#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include "app_common.h"

// Start control thread for background initialization
void start_control_thread(AppState* state);

// Stop control thread
void stop_control_thread(void);

#endif // CONTROL_THREAD_H