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
      :sp(0)
      ,pc(0)
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
      
      void setProgramCounter(Counter counter)
      {
         pc = counter;
      }
      
      Counter getProgramCounter()
      {
         return pc;
      }

      void skip()
      {
         pc += 2;
      }
   
      Stack::size_type sp;
      Stack stack;
      Registers V;
   private:
 
      Memory memory;
      Counter pc;

      void clearMemory()
      { 
         for(size_t i{}; i < chip8_fontset.size() ; i++)
         {
            memory[i] = chip8_fontset[i];		
         }
      }

   };

   // We are going to capture with the lambda so we need a std::function
   using OpcodeRunner      = std::function<void(Opcode)>;
   using OpcodeExtractor   = std::function<Register(Opcode)>;
   
   void nop(Opcode){}
  
   Register nnn(Opcode opcode)
   {
      return opcode & 0x0FFF;
   }
   Register kk(Opcode opcode)
   {
      return opcode & 0x00FF;
   }
   
   Register x(Opcode opcode)
   {
      return opcode & 0x0F00;
   }

   Register y(Opcode opcode)
   {
      return opcode & 0X00F0;
   }
}
class Chip8::Pimpl
{
public:
   Pimpl()
   :machine() // I know it's not needed but is good to be consistent
   ,Vx(V(&x)) // alias
   ,Vy(V(&y)) // alias
   ,opcodes{{
      nop, 
      jumpTo(nnn), 
      callTo(nnn),
      skipIfEquals(Vx, kk),
      skipIfNotEquals(Vx, kk),
      skipIfEquals(Vx, Vy),
      nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
   }}
   {}
   
   OpcodeExtractor V(OpcodeExtractor extractor)
   {
      return [&](Opcode opcode)
      {
         return machine.V[extractor(opcode)];
      };
   }

   OpcodeRunner skipIfEquals(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         if (lhs(opcode) == rhs(opcode)) machine.skip();
      };
   }
   
   OpcodeRunner skipIfNotEquals(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         if (lhs(opcode) != rhs(opcode)) machine.skip();
      };
   }

   OpcodeRunner jumpTo(OpcodeExtractor extractor)
   {
      return [&](Opcode opcode)
      {
         machine.setProgramCounter(extractor(opcode));   
      };
   }

   OpcodeRunner callTo(OpcodeExtractor extractor)
   {
      return [&](Opcode opcode)
      {
         // Increment stack pointer
         ++ machine.sp;
         
         // Puts the program counter on the top of the stack
         machine.stack[0] = machine.getProgramCounter();
         
         machine.setProgramCounter(extractor(opcode));     
      };
   }

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
      opcodes[mask];
      std::cout << opcode << std::endl;
   }
   
   void setKeys()
   {
   }


private:

   Machine machine;
   
   OpcodeExtractor Vx;
   OpcodeExtractor Vy;

   std::array<OpcodeRunner, 35> opcodes;
    
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




