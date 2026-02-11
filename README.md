<img width="1356" height="737" alt="CHIP8" src="https://github.com/user-attachments/assets/4639e4cd-7d0d-4fff-a90b-d8d380819a64" />

# CHIP-8 Emulator

CHIP-8 Emulator/Interpreter created using C and SDL3.

This is intended as a learning project.

## Building
Install SDL3 in your system first.

Run `make` in the project directory.

## Running
```sh
./main <rom file>
```

### References
- [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [CHIP-8 - Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
- [Learn Emulation with CHIP-8!](https://youtu.be/7HVXBzsujyc?si=fty30nBlkNFNxtBr)
- [CHIP-8 Emulator (C / SDL2)](https://youtube.com/playlist?list=PLT7NbkyNWaqbyBMzdySdqjnfUFxt8rnU_&si=FnmHspUsE_b0ESvN)

## Keybinds
F1 - Show grid

```
| 1 | 2 | 3 | C |            | 1 | 2 | 3 | 4 |
| 4 | 5 | 6 | D |            | Q | W | E | R |
| 7 | 8 | 9 | E |     =      | A | S | D | F |
| A | 0 | B | F |            | Z | X | C | V |
  CHIP-8 Keypad                  Keyboard
```

## Some images

<img width="1356" height="737" alt="Tetris" src="https://github.com/user-attachments/assets/c2409400-7678-44f3-9bc9-259569904355" />

Tetris

<img width="1356" height="737" alt="Pong" src="https://github.com/user-attachments/assets/d9c134df-69a3-4f7b-8f29-977b10936039" />

Pong

## Opcodes Implemented
- [ ] 0NNN (Not Applicable)
- [x] 00E0
- [x] 00EE
- [x] 1NNN
- [x] 2NNN
- [x] 3XNN
- [x] 4XNN
- [x] 5XY0
- [x] 6XNN
- [x] 7XNN
- [x] 8XY0
- [x] 8XY1
- [x] 8XY2
- [x] 8XY3
- [x] 8XY4
- [X] 8XY5
- [X] 8XY6
- [x] 8XY7
- [x] 8XYE
- [x] 9XY0
- [x] ANNN
- [x] BNNN
- [x] CXNN
- [x] DXYN
- [x] EX9E
- [x] EXA1
- [x] FX07
- [x] FX0A
- [x] FX15
- [X] FX18
- [x] FX1E
- [x] FX29
- [x] FX33
- [x] FX55
- [x] FX65
