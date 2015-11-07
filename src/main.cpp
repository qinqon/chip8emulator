#include <array>
#include <bitset>
#include <future>
#include <iostream>
#include <getopt.h>
#include <unordered_map>

#include "Chip8.h" 
#include "Display.h"

//Keypad                   Keyboard
//+-+-+-+-+                +-+-+-+-+
//|1|2|3|C|                |1|2|3|4|
//+-+-+-+-+                +-+-+-+-+
//|4|5|6|D|                |Q|W|E|R|
//+-+-+-+-+       =>       +-+-+-+-+
//|7|8|9|E|                |A|S|D|F|
//+-+-+-+-+                +-+-+-+-+
//|A|0|B|F|                |Z|X|C|V|
//+-+-+-+-+                +-+-+-+-+

std::map<sf::Keyboard::Key, Key> sfmlToChip9Key = 
{
   {sf::Keyboard::Num1, Key::Num1},      
   {sf::Keyboard::Num2, Key::Num2},      
   {sf::Keyboard::Num3, Key::Num3},      
   {sf::Keyboard::Num4, Key::C},      
   {sf::Keyboard::Q,    Key::Num4},      
   {sf::Keyboard::W,    Key::Num5},      
   {sf::Keyboard::E,    Key::Num6},      
   {sf::Keyboard::R,    Key::D},      
   {sf::Keyboard::A,    Key::Num7},      
   {sf::Keyboard::S,    Key::Num8},      
   {sf::Keyboard::D,    Key::Num9},      
   {sf::Keyboard::F,    Key::E},      
   {sf::Keyboard::Z,    Key::A},      
   {sf::Keyboard::X,    Key::Num0},      
   {sf::Keyboard::C,    Key::B},      
   {sf::Keyboard::V,    Key::F},      
};

struct Options
{
   uint8_t cpu_rate = 0;
   std::string rom_file;
};

void setupInput()
{
}

void printUsage()
{
   std::cout << "Usage: chip8emulator --rom-file|-r 'ROM file' [--cpu-rate|-c 'rate' ]" << std::endl;
   exit(EXIT_FAILURE);
}

Options loadOptions(int argc, char** argv)
{
   Options options;
   int opt = 0;

   static struct option long_options[] = 
   {
      {"cpu-rate", required_argument,  0, 'c'},
      {"rom-file", required_argument,  0, 'r'},
      {"help",    no_argument,         0, 'h'},
      {0, 0, 0, 0}
   };
  
   int option_index = 0;
   while ((opt = getopt_long (argc, argv, "r:ch", long_options, &option_index)) != -1)
   {
      switch (opt) 
      {
         case 'c':
            options.cpu_rate = atoi(optarg);
            break;
         case 'r':
            options.rom_file = optarg;
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
   chip8.loadGame(options.rom_file);
   
   auto drawNeededCallback = [&]
   {
      chip8.emulateCycle();
      return chip8.drawNeeded();
   };

   auto drawCallback = [&]
   {
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
   };
   
   auto keyPressedCallback = [&]
   (sf::Keyboard::Key key)
   {
      try
      {
         chip8.pressKey(sfmlToChip9Key.at(key));
      }
      catch(...)
      {}
   };
 
   auto keyReleasedCallback = [&]
   (sf::Keyboard::Key key)
   {
      try
      {
         chip8.releaseKey(sfmlToChip9Key.at(key));
      }
      catch(...)
      {}
   };

   display.loop(drawNeededCallback, drawCallback, keyPressedCallback, keyReleasedCallback);

   return 0;
}
