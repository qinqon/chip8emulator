#include "Chip8.h"

#include <cstdio>
#include <random>
#include <unistd.h>
#include <termios.h>

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <unordered_map>

#include "Chip8Types.h"

#ifdef DEBUG 
#define D(runner) debugRunner(runner, #runner)
#else
#define D(runner) runner
#endif 

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
      ,I(0xFFFF)
      ,delayTimer(0)
      ,soundTimer(0)
      // At the old systems the emulator is at the beginning
      ,pc(0x200)
      {
         
         clear(stack);
         clear(memory);
         clear(graphics);
         clear(V);

         loadFonset();  
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
      std::random_device rd;
      Stack::size_type sp;
      Stack stack;
      Registers V;
      Counter I;
      Timer delayTimer;
      Timer soundTimer;
      Graphics graphics;
   private:
 
      Memory memory;
      Counter pc;
      
      template<typename T, size_t S>
      void clear(std::array<T, S>& output)
      {
         for(size_t i = 0; i < S; i++)
         {
            output[i] = 0x00;
         }
      }

      void loadFonset()
      {
         for (auto i = 0; i < chip8_fontset.size(); ++i)
         {
            memory[i] = chip8_fontset[i]; 
         }
      }

   };
   
   enum class OpcodeRunnerResult { SkippNeeded, SkippNotNeeded };

   // We are going to capture with the lambda so we need a std::function
   using OpcodeRunner      = std::function<OpcodeRunnerResult(Opcode)>;
   using OpcodeExtractor   = std::function<Opcode(Opcode)>;
   using Extractor         = std::function<Register()>;
   template<size_t S>
   using Opcodes           = std::array<OpcodeRunner, S>;
   using Mapping           = std::unordered_map<Register, OpcodeRunner>;
   
   Register lsb(Register value)
   {
      auto mask = ~(std::numeric_limits<Register>::max() - 1);
      return value & mask;
   }
   
   Register msb(Register value)
   {
      auto bits = std::numeric_limits<Register>::digits - 1;
      return value >> bits;
   }

   Opcode nnn(Opcode opcode)
   {
      return opcode & 0x0FFF;
   }
   Opcode kk(Opcode opcode)
   {
      return opcode & 0x00FF;
   }
    
   Opcode n(Opcode opcode)
   {
      return opcode & 0x000F;
   }

   Opcode x(Opcode opcode)
   {
      return (opcode & 0x0F00) >> 8;
   }

   Opcode y(Opcode opcode)
   {
      return (opcode & 0x00F0) >> 4;
   }
   
   // Wait for a key to be pressed and return the value
   Register keyPressed()
   {
      Register buf = 0;
      struct termios old = {0};
      if (tcgetattr(0, &old) < 0)
         perror("tcsetattr()");
      old.c_lflag &= ~ICANON;
      old.c_lflag &= ~ECHO;
      old.c_cc[VMIN] = 1;
      old.c_cc[VTIME] = 0;
      if (tcsetattr(0, TCSANOW, &old) < 0)
         perror("tcsetattr ICANON");
      if (read(0, &buf, 1) < 0)
         perror ("read()");
      old.c_lflag |= ICANON;
      old.c_lflag |= ECHO;
      if (tcsetattr(0, TCSADRAIN, &old) < 0)
         perror ("tcsetattr ~ICANON");
      return (buf);
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
   ,drawFlag(false)
   ,cpuRate(0)
   ,runner(withMask(0xF000, 12, Opcodes<26>
   {{
     D(withMask(0x00FF, 
         Mapping{
            {0x00E0, D(clearDisplay())}, 
            {0x00EE, D(returnFromSubroutine())}
         }
      )), 
      // 0x01
      D(jumpTo(nnn)), 
      D(callTo(nnn)),
      D(skipIfEquals(Vx, kk)),
      D(skipIfNotEquals(Vx, kk)),
      // 0x05
      D(skipIfEquals(Vx, Vy)),
      D(setToV(x, kk)),
      D(addToV(x, kk)),
      D(withMask(0x000F, 
         Opcodes<9>{{
            D(setToV(x, Vy)),
            D(orToV(x, Vy)), 
            D(andToV(x, Vy)),
            D(xorToV(x, Vy)),  
            D(addToV(x, Vy)),
            D(subtractToV(x, Vy)), 
            D(shiftRightToVx()), 
            D(subtractNumericToVxVy()), 
            D(shiftLeftToVx()),
      }})),
      D(skipIfNotEquals(Vx, Vy)),
      // 0x0A
      D(setTo(&Machine::I, nnn)),
      D(jumpTo(V0, nnn)),
      D(setToV(x, randomAnd(kk))),
      D(display(Vx, Vy, n)),
      D(withMask(0x0FF, 
         Mapping{
            {0x009E, D(skipIfPressedVx())},
            {0x00A1, D(skipIfNotPressedVx())}
         })), 
      D(withMask(0x0FF, 
         Mapping{
            {0x0007, D(setToV(x, From(&Machine::delayTimer)))},
            {0x000A, D(setToV(x, keyPressed))},
            {0x0015, D(setTo(&Machine::delayTimer, Vx))},
            {0x0018, D(setTo(&Machine::soundTimer, Vx))},
            {0x001E, D(addTo(&Machine::I, Vx))},
            {0x0029, D(setToF(Vx))},
            {0x0033, D(setToB(Vx))},
            {0x0055, D(storeToMemory(V0, Vx))},
            {0x0065, D(readFromMemory(V0, Vx))},
         })),
   }}))
   {} 

   void loadGame(const std::string& name)
   {
      std::ifstream file(name, std::ios::binary);
      if (file.is_open())
      {
         auto begin = std::istreambuf_iterator<char>(file);
         auto end = std::istreambuf_iterator<char>();
         
         // The game has to be loaded after the the poss 0x200
         std::copy(begin, end, &machine.getMemory()[0x200]);

         file.close();
      }
   }
   
   void setCpuRate(uint8_t rate)
   {
      cpuRate = rate;
   }
  
   void emulateCycle()
   {
      emulateCpuRate();
      
      drawFlag = false;

      Opcode opcode = machine.fetchOpcode();

#ifdef DEBUG

      std::cout << "opcode: " << std::hex << opcode << std::endl;

#endif

      auto result = runner(opcode);
      if (result == OpcodeRunnerResult::SkippNeeded)
      {
         machine.skip();
      }
   }
   
   void setKeys()
   {
      //TODO:
   }
   
   bool drawNeeded()
   {
      return drawFlag;
   }

   const Graphics& getGraphics() const
   {
      return machine.graphics;
   }

private:
   
   void emulateCpuRate()
   {   
      if (cpuRate > 0)
      {
         // It will sleep for the number of microseconds between cycles
         usleep(1000000/cpuRate);
      }
   }

   Machine machine;
   OpcodeExtractor Vx;
   OpcodeExtractor Vy;
   Extractor V0;
   bool drawFlag;
   uint8_t cpuRate;

   OpcodeRunner runner;
   

   OpcodeExtractor FromV(OpcodeExtractor extractor)
   {
      return [extractor, this](Opcode opcode)
      {
         return machine.V[extractor(opcode)];
      };
   }
    
   Extractor FromV(size_t index)
   {
      return [index, this]()
      {
         return machine.V[index];
      };
   }
   
   template<typename T> 
   Extractor From(T Machine::*attribute)
   {
      return [attribute, this]()
      {
         return machine.*attribute;
      };
   }
   
   OpcodeRunner debugRunner(OpcodeRunner runner, const std::string& message)
   {
      return [=](Opcode opcode)
      {
         std::cout << message << std::endl;
         return runner(opcode);
      };
   }

   template<size_t S>
   OpcodeRunner withMask(Opcode mask, Opcodes<S> runners)
   {
      return withMask(mask, 0 /*no shift*/, runners);
   }
   
   template<size_t S>
   OpcodeRunner withMask(Opcode mask, size_t shift, Opcodes<S> runners)
   {
      return [=](Opcode opcode)
      {
         auto instruction = (opcode & mask) >> shift;
         auto& runner = runners.at(instruction);
         return runner(opcode);
      };
   }

   OpcodeRunner withMask(Opcode mask, Mapping runners)
   {
      return [=](Opcode opcode)
      {
         auto instruction = opcode & mask;
         auto& runner = runners.at(instruction);
         return runner(opcode);
      };
   }
 
   OpcodeRunner clearDisplay()
   {
      return [this](Opcode opcode)
      {
         //TODO:
         drawFlag = true;
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
 
   OpcodeRunner returnFromSubroutine()
   {
      return [this](Opcode opcode)
      {
         //TODO:
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner setToF(OpcodeExtractor)
   {
      return [this](Opcode opcode)
      {
         //TODO
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner setToB(OpcodeExtractor)
   {
      return [this](Opcode opcode)
      {
         //TODO
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner storeToMemory(Extractor, OpcodeExtractor)
   {
      return [this](Opcode opcode)
      {
         //TODO
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner readFromMemory(Extractor, OpcodeExtractor)
   {
      return [this](Opcode opcode)
      {
         //TODO
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner skipIfEquals(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         if (lhs(opcode) == rhs(opcode)) machine.skip();
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
   
   OpcodeRunner skipIfNotEquals(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         if (lhs(opcode) != rhs(opcode)) machine.skip();
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner jumpTo(OpcodeExtractor pcExtractor)
   {
      return [pcExtractor, this](Opcode opcode)
      {
         auto pc = pcExtractor(opcode);
         machine.setProgramCounter(pc);   
         return OpcodeRunnerResult::SkippNotNeeded;
      };
   }

   OpcodeRunner jumpTo(Extractor extractor, OpcodeExtractor opcodeExtractor)
   {
      return [extractor, opcodeExtractor, this](Opcode opcode)
      {
         machine.setProgramCounter(extractor() + opcodeExtractor(opcode));   
         return OpcodeRunnerResult::SkippNotNeeded;
      };
   }


   OpcodeRunner callTo(OpcodeExtractor extractor)
   {
      return [extractor, this](Opcode opcode)
      {
         machine.skip();
         
         // Puts the program counter on the top of the stack
         machine.stack[0] = machine.getProgramCounter();
         
         machine.setProgramCounter(extractor(opcode));     
         return OpcodeRunnerResult::SkippNotNeeded;
      };
   }
   
   /*   
   Set Vx = random byte AND kk.

   The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx. See instruction 8xy2 for more information on AND.
   */
   OpcodeExtractor randomAnd(OpcodeExtractor rhsExtractor)
   {
      return [rhsExtractor, this](Opcode opcode)
      {
         auto rhs = rhsExtractor(opcode);
         std::mt19937 mt(machine.rd());
         std::uniform_int_distribution<>dist(0, 255);
         return dist(mt) & rhs;
      };
   }
   
   template<typename T>
   OpcodeRunner setTo(T Machine::*attribute, OpcodeExtractor opcodeExtractor)
   {
      return [attribute, opcodeExtractor, this](Opcode opcode)
      {
         machine.*attribute = opcodeExtractor(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner setToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         machine.V[lhs(opcode)] = rhs(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
 
   OpcodeRunner setToV(OpcodeExtractor lhs, Extractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         machine.V[lhs(opcode)] = rhs();
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
  
   template <typename T>
   OpcodeRunner addTo(T Machine::*attribute, OpcodeExtractor opcodeExtractor)
   {
      return [attribute, opcodeExtractor, this](Opcode opcode)
      {
         machine.*attribute += opcodeExtractor(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner addToV(OpcodeExtractor lhsExtractor, OpcodeExtractor rhsExtractor)
   {
      return [lhsExtractor, rhsExtractor, this](Opcode opcode)
      {
         auto lhs = lhsExtractor(opcode);
         auto rhs = rhsExtractor(opcode);
         auto V = machine.V[lhs];
         machine.V[lhs] = V + rhs;
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
  
   OpcodeRunner subtractToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         machine.V[lhs(opcode)] -= rhs(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
  
   OpcodeRunner orToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         machine.V[lhs(opcode)] or_eq rhs(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner andToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         machine.V[lhs(opcode)] and_eq rhs(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }


   OpcodeRunner xorToV(OpcodeExtractor lhs, OpcodeExtractor rhs)
   {
      return [lhs, rhs, this](Opcode opcode)
      {
         machine.V[lhs(opcode)] xor_eq rhs(opcode);
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
   // If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. 
   // Then Vx is divided by 2.
   OpcodeRunner shiftRightToVx()
   {
      return [&](Opcode opcode)
      {
         auto x = ::x(opcode);
         auto Vx = machine.V[x];
         machine.V[0xF] = ((lsb(Vx) == 1) ? 1 : 0);
         machine.V[x] /= 2;
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   // Set Vx = Vy - Vx, set VF = NOT borrow.
   // If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
   OpcodeRunner subtractNumericToVxVy()
   {
      return [&](Opcode opcode)
      {
         auto x = ::x(opcode);
         auto y = ::y(opcode);
         auto Vx = machine.V[x];
         auto Vy = machine.V[y];
         
         machine.V[0xF] = ((Vy > Vx) ? 1 : 0);
         machine.V[x] -= Vy;
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   // Set Vx = Vx SHL 1.
   // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. 
   // Then Vx is multiplied by 2.
   OpcodeRunner shiftLeftToVx()
   {
      return [&](Opcode opcode)
      {
         auto x = ::x(opcode);
         auto Vx = machine.V[x];
         machine.V[0xF] = ((msb(Vx) == 1) ? 1 : 0);
         machine.V[x] *= 2;
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   OpcodeRunner display(OpcodeExtractor VxEtractor, OpcodeExtractor VyExtractor, OpcodeExtractor nExtractor)
   {
      return [VxEtractor, VyExtractor, nExtractor, this](Opcode opcode)
      {

         auto x = VxEtractor(opcode);
         auto y = VyExtractor(opcode);
         auto height = nExtractor(opcode);
         machine.V[0xF] = 0;
         for (int yoffset = 0; yoffset < height; yoffset ++)
         {
            auto pixel = machine.getMemory()[machine.I + yoffset];
            for(int xoffset = 0; xoffset < 8; xoffset ++)
            {  
               auto xoffset_mask = 0x80 >> xoffset;
               if((pixel & xoffset_mask) != 0)
               {
                  auto graphic_index = x + xoffset + ((y + yoffset) * 64);
                  auto previous_value = machine.graphics[graphic_index];
                  if(previous_value == 1)
                  {
                     machine.V[0xF] = 1;                                 
                  }
                  machine.graphics[graphic_index] = previous_value ^ 1;
               }
            }
         }
         drawFlag = true;
         return OpcodeRunnerResult::SkippNeeded;
      };
   }
   
   // Skip next instruction if key with the value of Vx is pressed.
   // Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
   OpcodeRunner skipIfPressedVx()
   {
      return [&](Opcode opcode)
      {
         auto Vx = machine.V[::x(opcode)];
         if (Vx == keyPressed())
         {
            machine.skip();
         }
         return OpcodeRunnerResult::SkippNeeded;
      };
   }

   // Skip next instruction if key with the value of Vx is not pressed.
   // Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
   OpcodeRunner skipIfNotPressedVx()
   {
      return [&](Opcode opcode)
      {
         auto Vx = machine.V[::x(opcode)];
         if (Vx != keyPressed())
         {
            machine.skip();
         }
         return OpcodeRunnerResult::SkippNeeded;
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
Chip8::setCpuRate(uint8_t rate)
{
   pimpl->setCpuRate(rate);
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

bool
Chip8::drawNeeded()
{
   return pimpl->drawNeeded();
}

const Graphics&
Chip8::getGraphics() const
{
   return pimpl->getGraphics();
}
