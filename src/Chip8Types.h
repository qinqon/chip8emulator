#ifndef _CHIP8TYPES_HH_
#define _CHIP8TYPES_HH_

// 16 bits are needed for the opcodes
using Opcode = unsigned short;

// Chip 8 has 8 bits general purpose registers;
using Register = unsigned char;

using Counter = unsigned short;

using Timer = unsigned char;

// The Chip 8 has 4K memory in total
using Memory = std::array<Register, 4096>;

// We don't need std::stack,  it uses dynamic memory;
using Stack = std::array<Counter, 16>;

// CPU registers: named V0,V1 up 
// to VE. The 16th register is used  for the ‘carry flag’.
using Registers = std::array<Register, 16>;

#endif // _CHIP8TYPES_HH_
