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

      case SDLK_1:
        chip8_set_key(chip8, 0x1, true);
        break;
      case SDLK_2:
        chip8_set_key(chip8, 0x2, true);
        break;
      case SDLK_3:
        chip8_set_key(chip8, 0x2, true);
        break;
      case SDLK_4:
        chip8_set_key(chip8, 0xC, true);
        break;

      case SDLK_Q:
        chip8_set_key(chip8, 0x4, true);
        break;
      case SDLK_W:
        chip8_set_key(chip8, 0x5, true);
        break;
      case SDLK_E:
        chip8_set_key(chip8, 0x6, true);
        break;
      case SDLK_R:
        chip8_set_key(chip8, 0xD, true);
        break;

      case SDLK_A:
        chip8_set_key(chip8, 0x7, true);
        break;
      case SDLK_S:
        chip8_set_key(chip8, 0x8, true);
        break;
      case SDLK_D:
        chip8_set_key(chip8, 0x9, true);
        break;
      case SDLK_F:
        chip8_set_key(chip8, 0xE, true);
        break;

      case SDLK_Z:
        chip8_set_key(chip8, 0xA, true);
        break;
      case SDLK_X:
        chip8_set_key(chip8, 0x0, true);
        break;
      case SDLK_C:
        chip8_set_key(chip8, 0xB, true);
        break;
      case SDLK_V:
        chip8_set_key(chip8, 0xF, true);
        break;

      default:
        break;
      }
    } else if (event.type == SDL_EVENT_KEY_UP) {
      switch (event.key.key) {
      case SDLK_1:
        chip8_set_key(chip8, 0x1, false);
        break;
      case SDLK_2:
        chip8_set_key(chip8, 0x2, false);
        break;
      case SDLK_3:
        chip8_set_key(chip8, 0x2, false);
        break;
      case SDLK_4:
        chip8_set_key(chip8, 0xC, false);
        break;

      case SDLK_Q:
        chip8_set_key(chip8, 0x4, false);
        break;
      case SDLK_W:
        chip8_set_key(chip8, 0x5, false);
        break;
      case SDLK_E:
        chip8_set_key(chip8, 0x6, false);
        break;
      case SDLK_R:
        chip8_set_key(chip8, 0xD, false);
        break;

      case SDLK_A:
        chip8_set_key(chip8, 0x7, false);
        break;
      case SDLK_S:
        chip8_set_key(chip8, 0x8, false);
        break;
      case SDLK_D:
        chip8_set_key(chip8, 0x9, false);
        break;
      case SDLK_F:
        chip8_set_key(chip8, 0xE, false);
        break;

      case SDLK_Z:
        chip8_set_key(chip8, 0xA, false);
        break;
      case SDLK_X:
        chip8_set_key(chip8, 0x0, false);
        break;
      case SDLK_C:
        chip8_set_key(chip8, 0xB, false);
        break;
      case SDLK_V:
        chip8_set_key(chip8, 0xF, false);
        break;

      default:
        break;
      }
    }
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
