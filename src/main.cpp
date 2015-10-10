//#include 
//#include   // OpenGL graphics and input
#include "Chip8.h" // Your cpu core implementation
 

void setupGraphics()
{
}

void setupInput()
{
}

void drawGraphics()
{
}

int main(int argc, char **argv) 
{
  // Set up render system and register input callbacks
  setupGraphics();
  setupInput();
 
  // Initialize the Chip8 system and load the game into the memory  
  Chip8 myChip8;
  
  myChip8.loadGame("pong");
 
  // Emulation loop
  for(;;)
  {
    // Emulate one cycle
    myChip8.emulateCycle();
 
    // If the draw flag is set, update the screen
    if(myChip8.drawFlag)
      drawGraphics();
 
    // Store key press state (Press and Release)
    myChip8.setKeys();	
  }
 
  return 0;
}
