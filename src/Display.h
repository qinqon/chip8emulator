#ifndef _DISPLAY_H_ 
#define _DISPLAY_H_

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <functional>

#include "Chip8Types.h"

class Display
{
public:
   using CycleCallback = std::function<std::pair<bool, bool>(void)>;
   using DrawingCallback = std::function<void(void)>;
   struct KeyboardCallbacks
   {
      std::function<void(sf::Keyboard::Key)> keyPressed;
      std::function<void(sf::Keyboard::Key)> keyReleased;
   };

   struct TouchpadCallbacks
   {
      std::function<void(const sf::Event::TouchEvent&)> touchBegan;
      std::function<void(const sf::Event::TouchEvent&)> touchEnded;
      std::function<void(const sf::Event::TouchEvent&)> touchMoved;
   };  

   Display();

   void drawPixel(size_t x, size_t y);
   void loop(
         CycleCallback,
         DrawingCallback,
         KeyboardCallbacks,
         TouchpadCallbacks
   );
private:
   sf::RenderWindow window;
   sf::VertexArray vertexArray;
   sf::SoundBuffer beepBuffer;
   sf::Sound beep;
   float pixelHigh;
   float pixelWidth;
};

#endif // _DISPLAY_H_
