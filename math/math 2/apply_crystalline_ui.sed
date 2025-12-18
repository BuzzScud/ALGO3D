# Apply Crystalline UI borders to visualization area
/\/\/ Draw background/,/SDL_RenderDrawRect.*viz_area_rect/ {
    s|// Draw background|// CRYSTALLINE UI: Draw background with proper panel styling|
    /SDL_RenderDrawRect.*viz_area_rect/a\
       \
       // CRYSTALLINE UI: Outer border (3px, lighter)\
       SDL_Color outer_border = {60, 60, 70, 255};\
       for (int i = 0; i < 3; i++) {\
           SDL_Rect outer = {viz_area_rect.x - i, viz_area_rect.y - i, \
                            viz_area_rect.w + 2*i, viz_area_rect.h + 2*i};\
           SDL_SetRenderDrawColor(renderer, outer_border.r, outer_border.g, outer_border.b, 255);\
           SDL_RenderDrawRect(renderer, &outer);\
       }\
       \
       // CRYSTALLINE UI: Inner border (2px, darker)\
       SDL_Color inner_border = {30, 30, 40, 255};\
       for (int i = 0; i < 2; i++) {\
           SDL_Rect inner = {viz_area_rect.x + 3 + i, viz_area_rect.y + 3 + i,\
                            viz_area_rect.w - 6 - 2*i, viz_area_rect.h - 6 - 2*i};\
           SDL_SetRenderDrawColor(renderer, inner_border.r, inner_border.g, inner_border.b, 255);\
           SDL_RenderDrawRect(renderer, &inner);\
       }
    /SDL_RenderDrawRect.*viz_area_rect/d
    /\/\/ Draw border/d
}