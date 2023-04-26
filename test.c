#include <SDL2/SDL.h>
#include <stdbool.h>

#define MAP_SIZE 100
#define WINDOW_SIZE 800

// Given a value between 0 and 255, returns a color between blue and red
SDL_Color get_heatmap_color(int value) {
    SDL_Color color = {0, 0, 0, 0};
    if (value <= 127) {
        color.r = (Uint8)(2 * value);
        color.g = (Uint8)(2 * value);
        color.b = (Uint8)(255 - 2 * value);
    } else {
        color.r = (Uint8)(255 - 2 * (value - 128));
        color.g = (Uint8)(255 - 2 * (value - 128));
        color.b = (Uint8)(2 * (value - 128));
    }
    return color;
}

// Given an integer array of size MAP_SIZE x MAP_SIZE, draws a heatmap to the screen
void draw_heatmap(int map[][MAP_SIZE], SDL_Renderer *renderer) {
    SDL_Rect rect;
    rect.w = rect.h = WINDOW_SIZE / MAP_SIZE;
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            SDL_Color color = get_heatmap_color(map[i][j]);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            rect.x = j * rect.w;
            rect.y = i * rect.h;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window and renderer
    SDL_Window *window = SDL_CreateWindow("Heatmap", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE,
                                          WINDOW_SIZE, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Create a 2D int array to store the heatmap data
    int map[MAP_SIZE][MAP_SIZE] = {0};
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            // Populate the array with some data (e.g., random values)
            map[i][j] = rand() % 256;
        }
    }

    // Draw the heatmap to the screen
    draw_heatmap(map, renderer);
    SDL_RenderPresent(renderer);

    // Wait for user to close the window
    SDL_Event event;
    while (true) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            break;
        }
    }

    // Cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
