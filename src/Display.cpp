#include "Display.h"

#include <array>
#include <iostream>

namespace 
{
   static const int8_t PIXEL_SIZE = 20;
}

Display::Display()
// Each pixel will be the 10x10
:window(sf::VideoMode(ScreenXLimit * PIXEL_SIZE, ScreenYLimit * PIXEL_SIZE), "Chip-8 emulator")
{}

void
Display::drawPixel(int8_t x, int8_t y)
{
   sf::RectangleShape shape({PIXEL_SIZE, PIXEL_SIZE});
   shape.setFillColor(sf::Color(100, 250, 50));
   shape.setPosition(x * PIXEL_SIZE, y * PIXEL_SIZE);
   window.draw(shape); 
}

void
Display::loop(std::function<bool(void)> drawNeeded, std::function<void(void)> doDrawing)
{
   while (window.isOpen())
   {
      sf::Event event;
      while (window.pollEvent(event))
      {
         if (event.type == sf::Event::Closed)
         {
            window.close();
         }
      }
      
      if (drawNeeded())
      {
         window.clear();
         doDrawing();
         window.display();
      }
   }
}

