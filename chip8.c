#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"

/* CHIP-8 fontset (0–F) */
static const uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_clear_display(chip8_t *chip8) {
  memset(chip8->display, false, sizeof(chip8->display));
}

void chip8_init(chip8_t *chip8) {
  memset(chip8, 0, sizeof(chip8_t));

  chip8->PC = 0x200; // Program entry point

  for (int i = 0; i < 80; i++) {
    chip8->ram[i] = chip8_fontset[i];
  }

  chip8_clear_display(chip8);

  // Used for CXNN opcode
  srand(time(NULL));
}

bool chip8_load_rom(chip8_t *chip8, const char *filename) {
  FILE *rom = fopen(filename, "rb");
  if (!rom) {
    perror("fopen");
    return false;
  }

  fseek(rom, 0, SEEK_END);
  size_t size = ftell(rom);
  rewind(rom);

  if (size > (CHIP8_MEMORY_SIZE - 0x200)) {
    fclose(rom);
    return false;
  }

  // Load rom into entry point
  fread(&chip8->ram[0x200], 1, size, rom);
  fclose(rom);

  return true;
}

void chip8_set_key(chip8_t *chip8, uint8_t key, bool pressed) {
  if (key > CHIP8_NUM_KEYS) {
    return;
  }

  chip8->keypad[key] = pressed;
}

void chip8_cycle(chip8_t *chip8) {
  /* Fetch */
  // Get the first two bytes and combine to get the opcode
  uint16_t opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];

  chip8->PC += 2;

  /* Decode */
  uint16_t NNN = opcode & 0x0FFF;
  uint8_t NN = opcode & 0x00FF;
  uint8_t N = opcode & 0x000F;
  uint8_t X = (opcode & 0x0F00) >> 8;
  uint8_t Y = (opcode & 0x00F0) >> 4;

  /* Execute */
  switch (opcode & 0xF000) {
  case 0x0000:
    switch (opcode) {
    case 0x00E0:
      // 00E0 Clears the screen
      printf("Opcode 0x%04x: Clears the screen\n", opcode);
      chip8_clear_display(chip8);
      break;
    case 0x00EE:
      // 00EE Returns from a subroutine
      printf("Opcode 0x%04x: Returns from a subroutine\n", opcode);
      chip8->sp--;
      chip8->PC = chip8->stack[chip8->sp];
      break;
    }
    break;

  case 0x1000:
    // 1NNN Jumps to address NNN
    printf("Opcode %#04x: Jumps address to %#x\n", opcode, NNN);
    chip8->PC = NNN;
    break;

  case 0x2000:
    // 2NNN Calls subroutine at NNN
    printf("Opcode %#04x: Calls subroutine at %#x\n", opcode, NNN);
    chip8->stack[chip8->sp++] = chip8->PC;
    chip8->PC = NNN;
    break;

  case 0x3000:
    // 3XNN Skips the next instruction if V[X] == NN
    printf("Opcode %#04x: Skips the next instruction if V[%u] == %u\n", opcode,
           X, NN);
    if (chip8->V[X] == NN) {
      chip8->PC += 2;
    }
    break;

  case 0x4000:
    // 4XNN Skips the next instruction if V[X] != NN
    printf("Opcode %#04x: Skips the next instruction if V[%u] != %u\n", opcode,
           X, NN);
    if (chip8->V[X] != NN) {
      chip8->PC += 2;
    }
    break;

  case 0x5000:
    // 5XY0 Skips the next instruction if V[X] == V[Y]
    printf("Opcode %#04x: Skips the next instruction if V[%u] == V[%u]\n",
           opcode, X, Y);
    if (chip8->V[X] == chip8->V[Y]) {
      chip8->PC += 2;
    }
    break;

  case 0x6000:
    // 6XNN Sets VX to NN
    printf("Opcode %#04x: Sets V[%u] to %u\n", opcode, X, NN);
    chip8->V[X] = NN;
    break;

  case 0x7000:
    // 7XNN Adss NN to VX
    printf("Opcode %#04x: Adds %u to V[%u]\n", opcode, NN, X);
    chip8->V[X] += NN;
    break;

  case 0xA000:
    // ANNN Sets I to address NNN
    printf("Opcode %#04x: Sets I to %#x\n", opcode, NNN);
    chip8->I = NNN;
    break;

  case 0xB000:
    // BNNN Jumps to the address V[0] + NNN
    printf("Opcode %#04x: Jumps to the address V[0] + %u\n", opcode, NNN);
    chip8->PC = chip8->V[0] + NNN;
    break;

  case 0xC000: {
    // CXNN Sets VX to rand() & NN
    printf("Opcode %#04x: Sets V[%u] to rand() & %u\n", opcode, X, NN);
    uint8_t random_number =
        rand() % 256; // Generate random number from 0 to 255
    chip8->V[X] = random_number & NN;
    break;
  }

  case 0xD000: {
    // DXYN
    // Draws a sprite at coordinates (VX, VY) with a width of 8 and height of N
    // VF is set to 1 if any pixels are flipped from set to unset
    printf("Opcode %#04x: draw(V[%u], V[%u], %u)\n", opcode, X, Y, N);
    uint8_t x_coord = chip8->V[X] % CHIP8_SCREEN_WIDTH;
    uint8_t y_coord = chip8->V[Y] % CHIP8_SCREEN_HEIGHT;
    chip8->V[0xF] = 0;

    for (int row = 0; row < N; row++) {
      uint8_t sprite = chip8->ram[chip8->I + row];

      // 8 is the sprite width on the fonts
      for (int col = 0; col < 8; col++) {
        // Check if bit is on from most to least significant bit
        if (sprite & (0b10000000 >> col)) {
          // Get the display index
          uint8_t px = (x_coord + col);
          uint8_t py = (y_coord + row);
          int index = py * CHIP8_SCREEN_WIDTH + px;

          // Sprite pixel is on and display pixel is on
          if (chip8->display[index] == 1) {
            chip8->V[0xF] = 1;
          }

          // Toggles the display pixel
          chip8->display[index] ^= 1;
        }
      }
    }

    break;
  }

  case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x0007:
      // FX07 Sets VX to the delay timer
      printf("Opcode %#04x: Sets V[%u] to the delay timer\n", opcode, X);
      chip8->V[X] = chip8->delay_timer;
      break;
    case 0x0015:
      // FX15 Sets the delay timer to VX
      printf("Opcode %#04x: Sets the delay timer to V[%u]\n", opcode, X);
      chip8->delay_timer = chip8->V[X];
      break;
    default:
      fprintf(stderr, "Unknown opcode: %#04x\n", opcode);
      break;
    }
    break;

  default:
    fprintf(stderr, "Unknown opcode: %#04x\n", opcode);
    break;
  }

  // Decrement timers
  if (chip8->delay_timer > 0) {
    chip8->delay_timer--;
  }
  if (chip8->sound_timer > 0) {
    chip8->sound_timer--;
  }
}
