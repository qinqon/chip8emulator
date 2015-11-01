#include <array>
#include <iostream>
#include <getopt.h>

#include "Chip8.h" 
#include "Display.h"

struct Options
{
   uint8_t cpu_rate = 0;
};

void setupInput()
{
}

void printUsage()
{
   std::cout << "Usage: chip8emulator [--cpurate|-r rate ]" << std::endl;
   exit(EXIT_FAILURE);
}

Options loadOptions(int argc, char** argv)
{
   Options options;
   int opt = 0;

   static struct option long_options[] = 
   {
      {"cpu-rate", required_argument,   0, 'r'},
      {"help",    no_argument,         0, 'h'},
      {0, 0, 0, 0}
   };
  
   int option_index = 0;
   while ((opt = getopt_long (argc, argv, "r:h", long_options, &option_index)) != -1)
   {
      switch (opt) 
      {
         case 'r':
            options.cpu_rate = atoi(optarg);
            break;
         case 'h':
            printUsage();
            break;
         default:
            throw std::invalid_argument(std::string(reinterpret_cast<char*>(&opt)));
         
      }
   }

   return options;
}

int main(int argc, char **argv) 
{
   Options options;
   try
   {
      options = loadOptions(argc, argv);
   }
   catch (std::invalid_argument e)
   {
      std::cerr << "Invalid argument " << e.what() << std::endl;
      printUsage();
   }


   Display display;
   setupInput();

   Chip8 chip8;
   
   chip8.setCpuRate(options.cpu_rate);
   chip8.loadGame("GAMES/MAZE");

   display.loop(
      [&]{
         chip8.emulateCycle();
         return chip8.drawNeeded();
      },
      [&]{
         for (size_t y = 0; y < ScreenYLimit; y++)
         {
            size_t y_offset = y * ScreenXLimit;
            for (size_t x = 0; x < ScreenXLimit; x++)
            {
               auto bits = std::bitset<8>(chip8.getGraphics().at(y_offset + x));
               for (size_t i = 0; i < 8; ++i)
               {
                  if (bits[i])
                  {
                     display.drawPixel(x, y);
                  }
               }
            }
         }
      }
   );

   return 0;
}
