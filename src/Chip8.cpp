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
      ,I(0)
      ,delayTimer(0)
      ,soundTimer(0)
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
      Counter I;
      Timer delayTimer;
      Timer soundTimer;
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
   using Extractor         = std::function<Register()>;
   template<size_t S>
   using Opcodes           = std::array<OpcodeRunner, S>;

   void nop(Opcode){}
  
   Register nnn(Opcode opcode)
   {
      return opcode & 0x0FFF;
   }
   Register kk(Opcode opcode)
   {
      return opcode & 0x00FF;
   }
    
   Register n(Opcode opcode)
   {
      return opcode & 0x000F;
   }

   Register x(Opcode opcode)
   {
      return opcode & 0x0F00;
   }

   Register y(Opcode opcode)
   {
      return opcode & 0X00F0;
   }
   
   Register keyPressed()
   {
      //TODO: Wait for a key to be pressed and return the value
      return 0;
   }

}
class Chip8::Pimpl
{

public:
   Pimpl()
   :machine() // I know it's not needed but is good to be consistent
   ,Vx(FromV(&x)) // alias
   ,Vy(FromV(&y)) // alias
   ,V0(FromV(0))  // alias
   ,runner(withMask(0xF000, Opcodes<35>
   {{
      //TODO: They are not consecutive
      withMask(0x0FFF, Opcodes<2>{{
         clearDisplay(),
         returnFromSubroutine(),
      }}), 
      jumpTo(nnn), 
      callTo(nnn),
      skipIfEquals(Vx, kk),
      skipIfNotEquals(Vx, kk),
      skipIfEquals(Vx, Vy),
      setToV(x, kk),
      addToV(x, kk),
      withMask(0x000F, Opcodes<9>{{
         setToV(x, Vy),
         orToV(x, Vy), 
         andToV(x, Vy),
         xorToV(x, Vy),  
         addToV(x, Vy),
         subtractToV(x, Vy), 
         shiftRightToV(x, Vy), 
         subtractNumericToV(x, Vy), 
         shiftLeftToV(x, Vy),
      }}),
      skipIfNotEquals(Vx, Vy),
      addTo(&Machine::I, nnn),
      jumpTo(V0, nnn),
      setToV(x, randomAnd(kk)),
      display(Vx, Vy, n),
      //TODO: They are not consecutive
      withMask(0x0FF, Opcodes<2>{{
         skipIfPressed(Vx), 
         skipIfNotPressed(Vx), 
      }}),
      //TODO: They are not consecutive
      withMask(0x0FF, Opcodes<9>{{
         setToV(x, From(&Machine::delayTimer)),
         setToV(x, keyPressed),
         setTo(&Machine::delayTimer, Vx),
         setTo(&Machine::soundTimer, Vx),
         addTo(&Machine::I, Vx),
         setToF(Vx),
         setToB(Vx),
         storeToMemory(V0, Vx),
         readFromMemory(V0, Vx),
      }}),
      nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
      nop, nop, nop, nop, nop, 
   }}))
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
      runner(opcode);
      std::cout << opcode << std::endl;
   }
   
   void setKeys()
   {
   }


private:

   Machine machine;
   OpcodeExtractor Vx;
   OpcodeExtractor Vy;
   Extractor V0;

   OpcodeRunner runner;
    
   OpcodeExtractor FromV(OpcodeExtractor extractor)
   {
      return [&](Opcode opcode)
      {
         return machine.V[extractor(opcode)];
      };
   }
    
   Extractor FromV(size_t index)
   {
      return [&]()
      {
         return machine.V[index];
      };
   }
   
   template<typename T> 
   Extractor From(T Machine::*attribute)
   {
      return [&]()
      {
         return machine.*attribute;
      };
   }
 
   template<size_t S>
   OpcodeRunner withMask(Opcode mask, Opcodes<S> runners)
   {
      return [&](Opcode opcode)
      {
         auto instruction = opcode & mask;
         runners[instruction](opcode);
      };
   }
   
   OpcodeRunner clearDisplay()
   {
      return [&](Opcode opcode)
      {
      };
   }
 
   OpcodeRunner returnFromSubroutine()
   {
      return [&](Opcode opcode)
      {
      };
   }

   OpcodeRunner setToF(OpcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner setToB(OpcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner storeToMemory(Extractor, OpcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner readFromMemory(Extractor, OpcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
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

   OpcodeRunner jumpTo(Extractor extractor, OpcodeExtractor opcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         machine.setProgramCounter(extractor() + opcodeExtractor(opcode));   
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
   
   OpcodeExtractor randomAnd(OpcodeExtractor opcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO: add the random and the AND operation
         return opcodeExtractor(opcode);
      };
   }
   
   template<typename T>
   OpcodeRunner setTo(T Machine::*attribute, OpcodeExtractor opcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         machine.*attribute = opcodeExtractor(opcode);
      };
   }

   OpcodeRunner setToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] = rhs(opcode);
      };
   }
 
   OpcodeRunner setToV(OpcodeExtractor lhs, Extractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] = rhs();
      };
   }
  
   template <typename T>
   OpcodeRunner addTo(T Machine::*attribute, OpcodeExtractor opcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         machine.*attribute += opcodeExtractor(opcode);
      };
   }

   OpcodeRunner addToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] += rhs(opcode);
      };
   }
  
   OpcodeRunner subtractToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] -= rhs(opcode);
      };
   }
  
   OpcodeRunner orToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] or_eq rhs(opcode);
      };
   }

   OpcodeRunner andToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] and_eq rhs(opcode);
      };
   }


   OpcodeRunner xorToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         machine.V[lhs(opcode)] xor_eq rhs(opcode);
      };
   }

   OpcodeRunner shiftRightToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner subtractNumericToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner shiftLeftToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner display(OpcodeExtractor VxEtractor, OpcodeExtractor VyExtractor, OpcodeExtractor nExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }
   
   OpcodeRunner skipIfPressed(OpcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
   }

   OpcodeRunner skipIfNotPressed(OpcodeExtractor)
   {
      return [&](Opcode opcode)
      {
         //TODO
      };
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




