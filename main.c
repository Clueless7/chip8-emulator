#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

#define SCALE 20
#define WINDOW_WIDTH (CHIP8_SCREEN_WIDTH * SCALE)
#define WINDOW_HEIGHT (CHIP8_SCREEN_HEIGHT * SCALE)

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} sdl_t;

bool init(sdl_t *sdl) {
  if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
    SDL_Log("Error: SDL_Init %s\n", SDL_GetError());
    return false;
  }

  sdl->window =
      SDL_CreateWindow("CHIP8 Emulator", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!sdl->window) {
    SDL_Log("Error: SDL_CreateWindow %s\n", SDL_GetError());
    return false;
  }

  sdl->renderer = SDL_CreateRenderer(sdl->window, NULL);
  if (!sdl->renderer) {
    SDL_Log("Error: SDL_CreateRenderer %s\n", SDL_GetError());
    return false;
  }

  return true;
}

void handle_input(chip8_t *chip8, bool *should_run, bool *debug) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      *should_run = false;
    } else if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_F1:
        *debug = !*debug;
        break;
      default:
        break;
      }
    } else if (event.type == SDL_EVENT_KEY_UP) {
    }
  }
}

void cleanup(const sdl_t sdl) {
  SDL_DestroyRenderer(sdl.renderer);
  SDL_DestroyWindow(sdl.window);
  SDL_Quit();
}

void draw_debug_grid(const sdl_t sdl) {
  SDL_SetRenderDrawBlendMode(sdl.renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(sdl.renderer, 255, 255, 255, 128);
  for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++) {
    const int px = x * SCALE;
    SDL_RenderLine(sdl.renderer, px, 0, px, WINDOW_HEIGHT);
  }
  for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++) {
    const int py = y * SCALE;
    SDL_RenderLine(sdl.renderer, 0, py, WINDOW_WIDTH, py);
  }
}

void draw_screen(chip8_t *chip8, const sdl_t sdl, bool *debug) {
  SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255); // BLACK
  SDL_RenderClear(sdl.renderer);

  SDL_SetRenderDrawColor(sdl.renderer, 255, 255, 255, 255); // WHITE

  // For each row
  for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++) {
    // Draw column
    for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++) {
      // If pixel is on then draw a colored rectangle
      if (chip8->display[y * CHIP8_SCREEN_WIDTH + x]) {
        const SDL_FRect r = {x * SCALE, y * SCALE, SCALE, SCALE};
        SDL_RenderFillRect(sdl.renderer, &r);
      }
    }
  }

  if (*debug) {
    draw_debug_grid(sdl);
  }

  SDL_RenderPresent(sdl.renderer);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <rom file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  sdl_t sdl = {0};
  if (!init(&sdl)) {
    exit(EXIT_FAILURE);
  }

  chip8_t chip8 = {0};
  chip8_init(&chip8);
  if (!chip8_load_rom(&chip8, argv[1])) {
    exit(EXIT_FAILURE);
  }

  bool should_run = true;
  bool debug = false;

  const Uint64 TARGET_FPS = 60;
  const Uint64 FRAME_DELAY_NS = 1e9 / TARGET_FPS; // 1ns / FPS

  // Main emulator loop
  while (should_run) {
    Uint64 frame_start = SDL_GetTicksNS();

    handle_input(&chip8, &should_run, &debug);

    chip8_cycle(&chip8);

    draw_screen(&chip8, sdl, &debug);

    // Need to target 16ms delay for 60 fps
    Uint64 frame_time = SDL_GetTicksNS() - frame_start;
    if (frame_time < FRAME_DELAY_NS) {
      SDL_DelayNS(FRAME_DELAY_NS - frame_time);
    }
  }

  cleanup(sdl);
  exit(EXIT_SUCCESS);
}
