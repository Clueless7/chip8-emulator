#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_STACK_SIZE 12
#define CHIP8_NUM_KEYS 16

typedef struct {
  // Memory
  uint8_t ram[CHIP8_MEMORY_SIZE]; // 4K bytes of ram

  // Registers
  uint8_t V[16]; // 16 8-bit registers V0-VF
  uint16_t I;    // 12-bit address register
  uint16_t PC;   // Program counter

  // Stack
  uint16_t stack[CHIP8_STACK_SIZE]; // Holds return addresses
  uint8_t sp;                       // Stack pointer which is index to stack

  // Timers
  uint8_t delay_timer; // Decrements at 60hz until 0
  uint8_t sound_timer; // Decrements at 60hz and play sound until 0

  // Input
  bool keypad[CHIP8_NUM_KEYS]; // 16 keys 0-F

  // Graphics
  bool display[CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT];

  // State
  bool draw_flag;
} chip8_t;

void chip8_init(chip8_t *chip8);
bool chip8_load_rom(chip8_t *chip8, const char *filename);
void chip8_cycle(chip8_t *chip8);
void chip8_set_key(chip8_t *chip8, uint8_t key, bool pressed);
void chip8_clear_display(chip8_t *chip8);

#endif
