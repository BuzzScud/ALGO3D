/*
 * Tab System Header
 * Defines the interface for all application tabs
 */

#ifndef TABS_H
#define TABS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Tab identifiers
typedef enum {
    TAB_VISUALIZATION = 0,
    TAB_CALCULATOR,
    TAB_LLM,
    TAB_TRAINING,
    TAB_RESEARCH,
    TAB_BENCHMARK,
    TAB_SETTINGS,
    TAB_COUNT
} TabID;

// Tab interface
typedef struct {
    TabID id;
    const char* name;
    void (*init)(void);
    void (*render)(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h);
    void (*update)(void);
    void (*handle_input)(SDL_Event* event);
    void (*cleanup)(void);
} Tab;

// LLM Tab functions
void llm_tab_init(void);
void llm_tab_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h);
void llm_tab_handle_input(SDL_Event* event);
void llm_tab_cleanup(void);

// Training Tab functions
void training_tab_init(void);
void training_tab_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h);
void training_tab_update(void);
void training_tab_handle_input(SDL_Event* event);
void training_tab_cleanup(void);

// Research Tab functions
void research_tab_init(void);
void research_tab_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h);
void research_tab_handle_input(SDL_Event* event);
void research_tab_cleanup(void);

// Benchmark Tab functions
void benchmark_tab_init(void);
void benchmark_tab_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h);
void benchmark_tab_update(void);
void benchmark_tab_handle_input(SDL_Event* event);
void benchmark_tab_cleanup(void);

// Tab system management
void tabs_init(void);
void tabs_switch_to(TabID tab_id);
TabID tabs_get_current(void);
const char* tabs_get_name(TabID tab_id);
void tabs_render_bar(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h);
void tabs_handle_click(int x, int y);
void tabs_cleanup(void);

#endif // TABS_H