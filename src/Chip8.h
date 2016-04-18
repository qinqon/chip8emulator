#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <functional>
#include <string>
#include <memory>

#include "Chip8Types.h"

class Chip8
{
public:
   Chip8();
   ~Chip8();
   void loadGame(const std::string& name);
   void loadGame(std::function<void(Register*)>);
   void setCpuRate(uint8_t);
   void emulateCycle();
   void pressKey(Key);
   void releaseKey(Key);
   bool drawNeeded();
   bool beepNeeded();
   const Graphics& getGraphics() const;
private:
   class Pimpl;
   std::unique_ptr<Pimpl> pimpl;

};

#endif // _CHIP8_H_
