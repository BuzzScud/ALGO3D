/**
 * Sphere Visualization Module Header
 */

#ifndef SPHERE_VISUALIZATION_H
#define SPHERE_VISUALIZATION_H

#include <SDL2/SDL.h>
#include "../app_common.h"

/**
 * Draw the 12 kissing spheres in a circular arrangement
 * Shows real-time activity, batch counts, and statistics
 */
void draw_sphere_visualization(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds);

/**
 * Draw detailed per-sphere statistics table
 */
void draw_sphere_stats_table(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds);

#endif // SPHERE_VISUALIZATION_H