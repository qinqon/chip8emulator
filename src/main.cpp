
#include <array>
#include <iostream>

#include "Chip8.h" 
#include "Display.h"

void setupInput()
{
}

int main(int argc, char **argv) 
{
   Display display;
   setupInput();

   Chip8 chip8;

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
