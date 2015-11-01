#ifndef _CHIP8TYPES_HH_
#define _CHIP8TYPES_HH_

// 16 bits are needed for the opcodes
using Opcode = uint16_t;

// We have 16 bits addresses
using Address = uint16_t;

// Chip 8 has 8 bits general purpose registers;
using Register = uint8_t;

using Counter = uint16_t;

using Timer = uint8_t;

// The Chip 8 has 4K memory in total
using Memory = std::array<Register, 4096>;

// We don't need std::stack,  it uses dynamic memory;
using Stack = std::array<Counter, 16>;

// CPU registers: named V0,V1 up 
// to VE. The 16th register is used  for the ‘carry flag’.
using Registers = std::array<Register, 16>;


//The graphics of the Chip 8 are black and white and the screen has a total of 2048 pixels (64 x 32).
const size_t ScreenXLimit = 64;
const size_t ScreenYLimit = 32;
using Graphics = std::array<Register, ScreenXLimit * ScreenYLimit>;

#endif // _CHIP8TYPES_HH_
