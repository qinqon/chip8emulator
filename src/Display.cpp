#include "Display.h"

#include <array>
#include <ctime>
#include <iostream>

Display::Display()
:window(sf::VideoMode::getFullscreenModes()[0], "Chip-8 emulator")
,pixelHigh(window.getSize().y /ScreenYLimit)
,pixelWidth(window.getSize().x / ScreenXLimit)
{
   if (not beepBuffer.loadFromFile("beep.wav"))
   {
      throw std::runtime_error("Beep wav file not found");
   }
   beep.setBuffer(beepBuffer);
}

void
Display::drawPixel(size_t x, size_t y)
{
   sf::RectangleShape rectangle(sf::Vector2f(pixelWidth, pixelHigh));
   rectangle.setPosition(x * pixelWidth, y * pixelHigh);
   rectangle.setFillColor(sf::Color::Green);  
   window.draw(rectangle);
}

void
Display::loop(CycleCallback doCycle, 
              DrawingCallback doDrawing, 
              KeyboardCallbacks keyboard, 
              TouchpadCallbacks touchpad
)
{
   sf::View view = window.getDefaultView();
   while (window.isOpen())
   {
      sf::Event event;
      while (window.pollEvent(event))
      {
         if (event.type == sf::Event::Closed)
         {
            window.close();
         }
         else if (event.type == sf::Event::Resized)
         {
            view.setSize(event.size.width, event.size.height);
            view.setCenter(event.size.width/2, event.size.height/2);
            window.setView(view);
         }
         else if (event.type == sf::Event::KeyPressed)
         {
            keyboard.keyPressed(event.key.code);
         }
         else if (event.type == sf::Event::KeyReleased)
         {
            keyboard.keyReleased(event.key.code);
         }
         else if (event.type == sf::Event::TouchBegan)
         {
            touchpad.touchBegan(event.touch);
         }
         else if (event.type == sf::Event::TouchEnded)
         {
            touchpad.touchEnded(event.touch);
         }
         else if (event.type == sf::Event::TouchMoved)
         {
            touchpad.touchMoved(event.touch);
         }
      }
      
      bool drawNeeded = false;
      bool beepNeeded = false;
      std::tie(drawNeeded, beepNeeded) = doCycle();
      if (drawNeeded)
      {
         window.clear(sf::Color::Black);
         vertexArray.clear();
         doDrawing();
         window.draw(vertexArray);
         window.display();
      }
      if(beepNeeded)
      { 
         std::cout << "BEEP !!!" << std::endl;
         beep.play();
      }
   }
}

