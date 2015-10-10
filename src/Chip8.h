#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <string>

class Chip8
{
public:
   Chip8();
   ~Chip8();
   void loadGame(const std::string& name);
   void emulateCycle();
   void setKeys();
   bool drawFlag;
private:
   class Pimpl;
   std::unique_ptr<Pimpl> pimpl;

};

#endif // _CHIP8_H_
