#ifndef _CHIP8TYPES_HH_
#define _CHIP8TYPES_HH_

// 16 bits are needed for the opcodes
using Opcode = unsigned short;

// Chip 8 has 8 bits general purpose registers;
using Register = unsigned char;

using Counter = unsigned short;

// The Chip 8 has 4K memory in total
using Memory = std::array<Register, 4096>;
#endif // _CHIP8TYPES_HH_
