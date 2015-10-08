#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <string>

class Chip8
{
public:
   void initialize();
   void loadGame(const std::string& name);
   void emulateCycle();
   void setKeys();
   bool drawFlag;
};


#endif // _CHIP8_H_
