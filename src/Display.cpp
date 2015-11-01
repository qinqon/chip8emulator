#include "Display.h"

#include <array>
#include <ctime>
#include <iostream>

namespace 
{
   static const int8_t PIXEL_SIZE = 20;
}

Display::Display()
// Each pixel will be the 10x10
:window(sf::VideoMode(ScreenXLimit * PIXEL_SIZE, ScreenYLimit * PIXEL_SIZE), "Chip-8 emulator")
,vertexArray(sf::Quads)
{}

void
Display::drawPixel(size_t x, size_t y)
{
   auto x1 = PIXEL_SIZE * x;
   auto y1 = PIXEL_SIZE * y;
   auto x2 = x1 + PIXEL_SIZE;
   auto y2 = y1 + PIXEL_SIZE;
   
   vertexArray.append({sf::Vector2f(x1, y1), sf::Color::Green});
   vertexArray.append({sf::Vector2f(x2, y1), sf::Color::Green});
   vertexArray.append({sf::Vector2f(x2, y2), sf::Color::Green});
   vertexArray.append({sf::Vector2f(x1, y2), sf::Color::Green});
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
         vertexArray.clear();
         doDrawing();
         window.draw(vertexArray);
         window.display();
      }
   }
}

