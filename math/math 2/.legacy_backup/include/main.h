#ifndef MAIN_H
#define MAIN_H

/*
 * main.h - Auto-generated header file
 * Source: main.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"

/* Local includes */
#include "app_common.h"

/* Function declarations */
void cleanup(AppState* state);
void expand_primes(AppState* state);
void handle_mouse_click(AppState* state, int x, int y);
void handle_input(AppState* state, SDL_Event* event);
void render(AppState* state);
int main(void);

#endif /* MAIN_H */