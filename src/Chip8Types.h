#ifndef _CHIP8TYPES_HH_
#define _CHIP8TYPES_HH_

#include <array>
#include <iostream>

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

// They key has 16 keys and two states
enum class KeyState {Pressed, Released};
enum class Key{
   Num1, Num2, Num3, C,
   Num4, Num5, Num6, D,
   Num7, Num8, Num9, E,
   A,    Num0, B,    F,
};

inline std::ostream& operator << (std::ostream& out, const Key& key)
{
   if (key == Key::Num1)
      out << "Num1";
   else if (key == Key::Num2)
      out << "Num2";
   else if (key == Key::Num3)
      out << "Num3";
   else if (key == Key::Num4)
      out << "Num4";
   else if (key == Key::Num5)
      out << "Num5";
   else if (key == Key::Num6)
      out << "Num6";
   else if (key == Key::Num7)
      out << "Num7";
   else if (key == Key::A)
      out << "A";
   else if (key == Key::B)
      out << "B";
   else if (key == Key::C)
      out << "C";
   else if (key == Key::D)
      out << "D";
   else if (key == Key::E)
      out << "E";
   else if (key == Key::F)
      out << "F";
   return out;
}

using Keypad = std::array<KeyState, 16>;



#endif // _CHIP8TYPES_HH_
