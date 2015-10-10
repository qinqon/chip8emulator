#include "Chip8.h"

#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include "Chip8Types.h"

namespace
{
   std::array<unsigned char, 80> chip8_fontset ={
   { 
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
   }};
   
   class Machine
   {
   public:
      Machine()
      :pc(0)
      {
         clearMemory();
      }
     
      Opcode fetchOpcode()
      {
         return memory[pc] << 8 | memory[pc + 1];
      }
      
      Memory::pointer getMemory()
      {
         return memory.data();
      }

 
      Memory::size_type getMemorySize()
      {
         return memory.size();
      }


   private:
 
      Counter pc;
      Memory memory;

      void clearMemory()
      { 
         for(size_t i{}; i < chip8_fontset.size() ; i++)
         {
            memory[i] = chip8_fontset[i];		
         }
      }

   };

   // It's going to be implemented with a functional view using composition
   using OpcodeExtractor   = Register (*)(Opcode);
   using OpcodeRunner      = void (*)(Machine&, Opcode);
   using OpcodeStatement   = bool (*)(Opcode);
   
   void nop(Machine&, Opcode)
   {
   }
   
   /*
   Register kk(Machine, Opcode)
   {
      return 0;
   }
   
   Register Vy(Machine, Opcode)
   {
      return 0;
   }

   Register Vx(Machine, Opcode)
   {
      return 0;
   }
   */


}
class Chip8::Pimpl
{
public:
   Pimpl()
   :machine() // I know it's not needed but is good to be consistent
   ,opcodeRunners{{
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
   }}
   {}
     
   void loadGame(const std::string& name)
   {
      std::ifstream file(name, std::ios::binary);
      if (file.is_open())
      {
         // Put the cursor at the beginning
         file.seekg(0, std::ios::beg);
         file.read(reinterpret_cast<char*>(machine.getMemory()), machine.getMemorySize());
         file.close();
      }
   }
   
  
   void emulateCycle()
   {
      Opcode opcode = machine.fetchOpcode();
      auto mask = opcode & 0xF000;
      opcodeRunners[mask];
      std::cout << opcode << std::endl;
   }
   
   void setKeys()
   {
   }


private:

   Machine machine;

   std::array<OpcodeRunner, 35> opcodeRunners;
    
   void skipInstruction()
   {
   }
 

};

Chip8::Chip8()
:pimpl(new Pimpl())
{}

Chip8::~Chip8() = default;

void 
Chip8::loadGame(const std::string& name)
{
   pimpl->loadGame(name);
}

void 
Chip8::emulateCycle()
{
   pimpl->emulateCycle();
}

void 
Chip8::setKeys()
{
   pimpl->setKeys();
}




