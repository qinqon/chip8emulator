#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <SFML/Graphics.hpp>

#include <functional>

#include "Chip8Types.h"

class Display
{
public:
   
   Display();

   void drawPixel(size_t x, size_t y);
   void loop(std::function<bool(void)>, std::function<void(void)>);
private:
   sf::RenderWindow window;
   sf::VertexArray vertexArray;
};

#endif // _DISPLAY_H_
