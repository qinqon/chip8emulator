#include <android/log.h>

#include "Display.h"
#include "Chip8.h"

#include <deque>
#include <iostream>

#include <SFML/System.hpp>

#define APPNAME "Chip8"


class androidbuf : public std::streambuf {
   public:
      enum { bufsize = 128 }; // ... or some other suitable buffer size
      androidbuf(const char* _tag) 
      :tag(_tag)
      { 
         this->setp(buffer, buffer + bufsize - 1); 
      }

   private:
      int overflow(int c)
      {
         if (c == traits_type::eof()) {
            *this->pptr() = traits_type::to_char_type(c);
            this->sbumpc();
         }
         return this->sync()? traits_type::eof(): traits_type::not_eof(c);
      }

      int sync()
      {
         int rc = 0;
         if (this->pbase() != this->pptr()) {
            char writebuf[bufsize+1];
            memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
            writebuf[this->pptr() - this->pbase()] = '\0';

            rc = __android_log_write(ANDROID_LOG_INFO, tag, writebuf) > 0;
            this->setp(buffer, buffer + bufsize - 1);
         }
         return rc;
      }

      char buffer[bufsize];
      const char* tag;
};

//TODO: Calculate the areas dynamically
std::deque<std::pair<sf::IntRect, Key>> pongTouchAreaToKey = 
{
   // Upper left area player1 up
   {{0, 0, 350, 350}, Key::Num1},

   // Button left area player1 down
   {{0, 350, 350, 350}, Key::Num4},

   // Upper right area player2 up
   {{0, 0, 0, 0}, Key::C},

   // Button right area player2 down
   {{0, 0, 0, 0}, Key::D},
};

int main(void)
{
   std::cout.rdbuf(new androidbuf(APPNAME));
   
#ifdef DEBUG
   std::cout << "Debug mode" << std::endl; 
#endif
   
   std::cout << "Starting Chip-8 emulator" << std::endl;
   Display display;
   Chip8 chip8;
   chip8.setCpuRate(250);
   chip8.loadGame([](Register* offset){
      sf::FileInputStream gameFile;
      if (not gameFile.open("PONG"))
      {
         throw std::invalid_argument("Cannot open game");
      }
      gameFile.read(offset, gameFile.getSize());
   });
   auto cycleCallback = [&]
   {
      chip8.emulateCycle();
      return std::make_pair(chip8.drawNeeded(), chip8.beepNeeded());
   };
   
   auto drawCallback = [&]
   {
      for (size_t y = 0; y < ScreenYLimit; y++)
      {
         size_t y_offset = y * ScreenXLimit;
         for (size_t x = 0; x < ScreenXLimit; x++)
         {
            auto bits = std::bitset<8>(chip8.getGraphics().at(y_offset + x));
            for (size_t i = 0; i < 8; ++i)
            {
               if (bits[i])
               {
                  display.drawPixel(x, y);
               }
            }
         }
      }
   };
   
   Display::KeyboardCallbacks keyboard;
   Display::TouchpadCallbacks touchpad;

   keyboard.keyPressed = 
   [&](sf::Keyboard::Key key)
   {
      std::cout << "Key pressed" << std::endl;
   };
   
   keyboard.keyReleased =
   [&](sf::Keyboard::Key key)
   {
      std::cout << "Key released" << std::endl;
   };
   
   touchpad.touchBegan = 
   [&](const sf::Event::TouchEvent& touch)
   {
      for (const auto& areaAndKey : pongTouchAreaToKey)
      {
         if (areaAndKey.first.contains(touch.x, touch.y))
         {
            chip8.pressKey(areaAndKey.second);
            std::cout << "Pressing key: " << areaAndKey.second << std::endl;
         }
      }
   };
   touchpad.touchEnded = 
   [&](const sf::Event::TouchEvent& touch)
   {
      for (const auto& areaAndKey : pongTouchAreaToKey)
      {
         if (areaAndKey.first.contains(touch.x, touch.y))
         {
            chip8.releaseKey(areaAndKey.second);
            std::cout << "Releasing key: " << areaAndKey.second << std::endl;
         }
      }
   };
   touchpad.touchMoved = 
   [&](const sf::Event::TouchEvent& touch)
   {
  
   };

   display.loop(
         cycleCallback, 
         drawCallback, 
         keyboard, 
         touchpad);
   return 0;   
}
