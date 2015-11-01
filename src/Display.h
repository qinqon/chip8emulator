#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <SFML/Graphics.hpp>

#include <functional>

#include "Chip8Types.h"

class Display
{
public:
   
   Display();

   void drawPixel(int8_t x, int8_t y);
   void loop(std::function<bool(void)>, std::function<void(void)>);
private:
   sf::RenderWindow window;
};

#endif // _DISPLAY_H_
