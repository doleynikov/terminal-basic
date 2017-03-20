/*
   ucBASIC is a lightweight BASIC-like language interpreter
   Copyright (C) 2016, 2017 Andrey V. Skvortsov <starling13@mail.ru>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "basic.hpp"

#include "basic_program.hpp"
#include "basic_arduinoio.hpp"

//#include "seriallight.hpp"

#if USESD
#include "basic_sdfs.hpp"
#endif

#if USEUTFT
#include "utft_stream.hpp"
#endif

#if USEMATH
#include "basic_math.hpp"
#endif

/**
   Instantiating modules
*/

#if USEUTFT
static UTFT	utft(CTE32HR, 38, 39, 40, 41);
static UTFTTerminal utftPrint(utft);
#endif

#if USESD
static BASIC::SDFSModule sdfs;
#endif

#if USEMATH
static BASIC::Math mathBlock;
#endif

#if USEARDUINOIO
static BASIC::ArduinoIO arduinoIo;
#endif

#include <Wire.h>
#include "LiquidCrystal_I2C.h"

class myLcd : public LiquidCrystal_I2C   // производный класс
{
    char screen[16]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    int8_t pos = 0;

  public:

    myLcd(uint8_t a, uint8_t b, uint8_t c) : LiquidCrystal_I2C (a, b, c)     {}

    inline size_t write(uint8_t value) {
      Serial.write(value);
      if (value != '\n' && value != '\r') {
        
        if (pos > 15) {
          scroll();      
          setCursor(0, 1);
          pos=0;
        }
        if (value == '\t')value = ' ';
        screen[pos++] = value;
        send(value, Rs);
      }
      else if (value == '\n') {scroll();}
      else if (value == '\r') {setCursor(0, 1);pos=0;}
      return 1;
    }
    
void    scroll() {
      int8_t tpos = pos;
      clear();
      setCursor(0, 0);
      for (int i = 0; i < 15; i++) {
        send(screen[i], Rs);
        screen[i]=' ';
      }
      pos=tpos;
      setCursor(tpos, 1);
    }

};
myLcd lcd(0x27, 16, 2);


static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE);
//static BASIC::Interpreter basic(Serial, Serial, program);
static BASIC::Interpreter basic(Serial, lcd, program);

void setup()
{
//  lcd.begin(16,2,0);
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 1);


#if USE_EXTMEM
  XMCRA |= 1ul << 7; // Switch ext mem iface on
  XMCRB = 0;
#endif
  Serial.begin(9600);
#if USEUTFT
  utftPrint.begin();
#endif



#if USEARDUINOIO
  basic.addModule(&arduinoIo);
#endif

#if USEMATH
  basic.addModule(&mathBlock);
#endif

#if USESD
  basic.addModule(&sdfs);
#endif
  basic.init();
}

void
loop()
{

  basic.step();

}

namespace BASIC
{

bool scanTable(const uint8_t *token, const uint8_t table[], uint8_t &index)
{
  uint8_t tokPos = 0, tabPos = 0;
  while (token[tokPos] != 0) {
    uint8_t c = pgm_read_byte(table);
    uint8_t ct = token[tokPos];
    if (c == 0)
      return (false);

    if (ct == c)
      ++tokPos, ++table;
    else if (ct + uint8_t(0x80) == c) {
      index = tabPos;
      if (token[++tokPos] != 0) {
        while ((pgm_read_byte(table++) & uint8_t(0x80)) ==
               0);
        ++tabPos, tokPos = 0;
      } else
        return (true);
    } else {
      if (c & uint8_t(0x80))
        c &= ~uint8_t(0x80);
      if (c > ct)
        return (false);
      else {
        while ((pgm_read_byte(table++) & uint8_t(0x80)) ==
               0);
        ++tabPos, tokPos = 0;
      }
    }
  }
  return (false);
}

}