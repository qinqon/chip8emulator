//#include 
//#include   // OpenGL graphics and input
#include "Chip8.h" // Your cpu core implementation

#include <array>
#include <iostream>

void setupGraphics()
{
}

void setupInput()
{
}

void drawGraphics(const Graphics& graphics)
{
   for (size_t y = 0; y < ScreenYLimit; y++)
   {
      size_t y_offset = y * ScreenXLimit;
      for (size_t x = 0; x < ScreenXLimit; x++)
      {
         std::cout << graphics.at(y_offset + x);
      }
      std::cout << std::endl;
   }
}

int main(int argc, char **argv) 
{
   // Set up render system and register input callbacks
   setupGraphics();
   setupInput();

   // Initialize the Chip8 system and load the game into the memory  
   Chip8 chip8;

   chip8.loadGame("GAMES/MAZE");

   // Emulation loop
   for(;;)
   {
      // Emulate one cycle
      chip8.emulateCycle();

      // If the draw flag is set, update the screen
      if (chip8.draw())
         drawGraphics(chip8.getGraphics());

      // Store key press state (Press and Release)
      chip8.setKeys();	
   }

   return 0;
}
