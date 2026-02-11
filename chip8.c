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
  if (key >= CHIP8_NUM_KEYS) {
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
    // 7XNN Adds NN to VX
    printf("Opcode %#04x: Adds %u to V[%u]\n", opcode, NN, X);
    chip8->V[X] += NN;
    break;

  case 0x8000:
    switch (opcode & 0x000F) {
    case 0x0000:
      // 8XY0 Sets VX to the value VY
      printf("Opcode %#04x: Sets V[%u] to the value V[%u]\n", opcode, X, Y);
      chip8->V[X] = chip8->V[Y];
      break;
    case 0x0001:
      // 8XY1 Sets VX to VX bitwise or VY
      printf("Opcode %#04x: Sets V[%u] to V[%u] bitwise or V[%u]\n", opcode, X,
             X, Y);
      chip8->V[X] |= chip8->V[Y];
      break;
    case 0x0002:
      // 8XY2 Sets VX to VX bitwise and VY
      printf("Opcode %#04x: Sets V[%u] to V[%u] bitwise and V[%u]\n", opcode, X,
             X, Y);
      chip8->V[X] &= chip8->V[Y];
      break;
    case 0x0003:
      // 8XY3 Sets VX to VX xor VY
      printf("Opcode %#04x: Sets V[%u] to V[%u] xor V[%u]\n", opcode, X, X, Y);
      chip8->V[X] ^= chip8->V[Y];
      break;
    case 0x0004:
      // 8XY4 Adds VY to VX, Sets VF to 1 if there's an overflow otherwise 0
      printf("Opcode %#04x: Adds V[%u] to V[%u], Sets VF to 1 if there's an "
             "overflow otherwise 0\n",
             opcode, Y, X);
      // uint16 to store more than 256
      uint16_t sum = chip8->V[X] + chip8->V[Y];

      chip8->V[0xF] = (sum > 255);
      // sum & 0xFF wraps sum to 255 e.g. 256 = 0, 257 = 1
      chip8->V[X] = sum & 0xFF;
      break;
    case 0x0005:
      // 8XY5 VY is subtracted from VX, Sets VF to 0 if theres an underflow
      // otherwise 1
      printf("Opcode %#04x: V[%u] is subtracted from V[%u], Sets VF to 0 if "
             "theres an underflow otherwise 1\n",
             opcode, Y, X);
      // If VX is larger than VY there is no underflow
      chip8->V[0xF] = (chip8->V[X] >= chip8->V[Y]);
      chip8->V[X] -= chip8->V[Y];
      break;
    case 0x0006:
      // 8XY6 Shifts VX to the right by 1, Sets VF to the least significant bit
      // of VX prior to shift
      printf("Opcode %#04x: Shifts V[%u] to the right by 1, Sets VF to the "
             "least significant bit of V[%u] prior to shift\n",
             opcode, X, X);
      chip8->V[0xF] = (chip8->V[X] & 1);
      chip8->V[X] >>= 1;
      break;
    case 0x0007:
      // 8XY7 Sets VX to VY - VX, VF is set to 1 if VY >= VX
      printf("Opcode %#04x: Sets V[%u] to V[%u] - V[%u], VF is set to 1 if "
             "V[%u] >= V[%u]\n",
             opcode, X, Y, X, Y, X);
      chip8->V[0xF] = (chip8->V[Y] >= chip8->V[X]);
      chip8->V[X] = chip8->V[Y] - chip8->V[X];
      break;
    case 0x000E:
      // 8XYE Shifts VX to the left by 1, Store most significant bit of VX to VF
      printf("Opcode %#04x: Shifts V[%u] to the left by 1, Store the most "
             "significant bit of V[%u] to VF\n",
             opcode, X, X);
      // Store most significant bit of VX to VF
      chip8->V[0xF] = (chip8->V[X] & 0b10000000) >> 7;
      chip8->V[X] <<= 1;
      break;
    default:
      fprintf(stderr, "\x1b[31mUnknown opcode: %#04x\x1b[0m\n", opcode);
      break;
    }
    break;

  case 0x9000:
    // 9XY0 Skips the next instruction if VX != VY
    printf("Opcode %#04x: Skips the next instruction if V[%u] != V[%u]\n",
           opcode, X, Y);
    if (chip8->V[X] != chip8->V[Y]) {
      chip8->PC += 2;
    }
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
          uint8_t px = (x_coord + col) % CHIP8_SCREEN_WIDTH;
          uint8_t py = (y_coord + row) % CHIP8_SCREEN_HEIGHT;
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

  case 0xE000:
    switch (opcode & 0x00FF) {
    case 0x009E:
      // EX9E Skips the next instruction if key() == VX
      printf("Opcode %#04x: Skips the next instruction if key() == V[%u]\n",
             opcode, X);
      if (chip8->keypad[chip8->V[X]]) {
        chip8->PC += 2;
      }
      break;
    case 0x00A1:
      // EXA1 Skips the next instruction if key() != VX
      printf("Opcode %#04x: Skips the next instruction if key() != V[%u]\n",
             opcode, X);
      if (!chip8->keypad[chip8->V[X]]) {
        chip8->PC += 2;
      }
      break;
    default:
      fprintf(stderr, "\x1b[31mUnknown opcode: %#04x\x1b[0m\n", opcode);
      break;
    }
    break;

  case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x0007:
      // FX07 Sets VX to the delay timer
      printf("Opcode %#04x: Sets V[%u] to the delay timer\n", opcode, X);
      chip8->V[X] = chip8->delay_timer;
      break;
    case 0x000A: {
      // FX0A Await key press then store to VX
      printf("Opcode %#04x: Await key press then store to V[%u]\n", opcode, X);
      bool key_found = false;

      for (uint8_t i = 0; i < CHIP8_NUM_KEYS; i++) {
        if (chip8->keypad[i]) {
          chip8->V[X] = i;
          key_found = true;
          break;
        }
      }

      if (!key_found) {
        chip8->PC -= 2;
      }

      break;
    }
    case 0x0015:
      // FX15 Sets the delay timer to VX
      printf("Opcode %#04x: Sets the delay timer to V[%u]\n", opcode, X);
      chip8->delay_timer = chip8->V[X];
      break;
    case 0x0018:
      // FX18 Sets the sound timer to VX
      printf("Opcode %#04x: Sets the sound timer to V[%u]\n", opcode, X);
      chip8->sound_timer = chip8->V[X];
      break;
    case 0x001E:
      // FX1E Adds VX to I
      printf("Opcode %#04x: Adds V[%u] to I\n", opcode, X);
      chip8->I += chip8->V[X];
      break;
    case 0x0029:
      // FX29 Sets I to the location of the sprite for the character in VX
      printf("Opcode %#04x: Sets I to the location of the sprite for the "
             "character in V[%u]\n",
             opcode, X);
      chip8->I = chip8->V[X] * 5;
      break;
    case 0x0033: {
      // FX33 Stores the decimal representation of VX with 100s digit to memory
      // location I 10s digit to I+1 and ones digit to I+3
      // E.g. if value is 210
      // I + 0 = 2
      // I + 1 = 1
      // I + 3 = 0
      printf(
          "Opcode %#04x: Stores binary coded decimal representation of V[%u]\n",
          opcode, X);
      uint8_t value = chip8->V[X];
      chip8->ram[chip8->I] = value / 100;
      chip8->ram[chip8->I + 1] = (value / 10) % 10;
      chip8->ram[chip8->I + 2] = value % 10;
      break;
    }
    case 0x0055:
      // FX55 Stores from V0 to VX (including VX) in memory, starting at address
      // I. The offset from I is increased by 1 for each value written
      printf("Opcode %#04x: reg_dump(V[%u], &I)\n", opcode, X);
      for (int i = 0; i <= X; i++) {
        chip8->ram[chip8->I + i] = chip8->V[i];
      }
      break;
    default:
      fprintf(stderr, "\x1b[31mUnknown opcode: %#04x\x1b[0m\n", opcode);
      break;
    }
    break;

  default:
    fprintf(stderr, "\x1b[31mUnknown opcode: %#04x\x1b[0m\n", opcode);
    break;
  }
}

void chip8_decrement_timers(chip8_t *chip8) {
  if (chip8->delay_timer > 0) {
    chip8->delay_timer--;
  }
  if (chip8->sound_timer > 0) {
    chip8->sound_timer--;
  }
}
