/*
 * ucBASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016, 2017 Andrey V. Skvortsov <starling13@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "basic.hpp"
//#include "arduino_logger.hpp"
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
 * Instantiating modules
 */

//********************************************************************

#include <Wire.h>
#include "MyLiquidCrystal_I2C.h"
#include "matrixKB.h"

class myLcd : public MyLiquidCrystal_I2C   // производный класс
{
  char screen[40]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '/*,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '*/};
  int8_t pos = 0;

  public:

    myLcd(uint8_t a, uint8_t b, uint8_t c) : MyLiquidCrystal_I2C (a, b, c)     {}

    inline size_t write(uint8_t value) {
      if (value == '\t')value = ' ';
      if (value != '\n' && value != '\r') {
        if (pos >= 40) {
          scroll();      
          setCursor(0, 1);
          pos=0;
        }
        screen[pos++] = value;
        send(value, Rs);
      }
      else if (value == '\n') {scroll();}
      else if (value == '\r') {setCursor(0, 1);pos=0;}
      return 1;
    }
 private:   
void    scroll() {
      int8_t tpos = pos;
      clear();
      setCursor(0, 0);
      for (int8_t i = 0; i < 40; i++) {
        send(screen[i], Rs);
        screen[i]=' ';
      }
      pos=tpos;
      setCursor(pos, 1);
    }

};

myLcd lcd(0x27, 16, 2);
matrixKB kb;

//********************************************************************



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

#if BASIC_MULTITERMINAL
static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic(Serial, Serial, program);
#if HAVE_HWSERIAL1
static BASIC::Interpreter::Program program1(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic1(Serial1, Serial1, program1);
#endif
#if HAVE_HWSERIAL2
static BASIC::Interpreter::Program program2(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic2(Serial2, Serial2, program2);
#endif
#if HAVE_HWSERIAL3
static BASIC::Interpreter::Program program3(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic3(Serial3, Serial3, program3);
#endif
#else
static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE);
#if USEUTFT
static BASIC::Interpreter basic(Serial, utftPrint, program);
#else
#ifdef ARDUINO
//static BASIC::Interpreter basic(SerialL, SerialL, program);
static BASIC::Interpreter basic(kb, lcd, program);
#else
static BASIC::Interpreter basic(Serial, Serial, program);
#endif
#endif // USEUTFT
#endif // BASIC_MULTITERMINAL

void
setup()
{
//******************************************************************************

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 1);
  kb.begin();

//******************************************************************************

#if USE_EXTMEM
	XMCRA |= 1ul<<7; // Switch ext mem iface on
	XMCRB = 0;
#endif
#ifdef ARDUINO
//	SerialL.begin(9600);
#else
	Serial.begin(9600);
#endif // ARDUINO
#if USEUTFT
	utftPrint.begin();
#endif

#if BASIC_MULTITERMINAL
#if HAVE_HWSERIAL1
	Serial1.begin(57600);
#endif
#if HAVE_HWSERIAL2
	Serial2.begin(57600);
#endif
#if HAVE_HWSERIAL3
	Serial3.begin(57600);
#endif
#endif

//	LOG_INIT(SerialL);

//	LOG_TRACE;

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
#if BASIC_MULTITERMINAL
#if HAVE_HWSERIAL1
	basic1.init();
#endif
#if HAVE_HWSERIAL2
	basic2.init();
#endif
#if HAVE_HWSERIAL3
	basic3.init();
#endif
#endif
}

void
loop()
{
//	LOG_TRACE;
	
	basic.step();
#if BASIC_MULTITERMINAL
#ifdef HAVE_HWSERIAL1
	basic1.step();
#endif
#ifdef HAVE_HWSERIAL2
	basic2.step();
#endif
#ifdef HAVE_HWSERIAL3
	basic3.step();
#endif
#endif
}
