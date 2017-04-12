#include "matrixKB.h"


#define KEY_ENTER 13
#define KEY_BRK 27 // пусть break будет кодом ESCAPE
#define SYMBOL_SHIFT 1
#define CAPS_SHIFT 2


// Keyboard
#define  ROWS  8
#define  COLS  5
char  charMap[4][ROWS][COLS] =
{
  {
    //0
    {'5', '4', '3', '2', '1'},
    {'T', 'R', 'E', 'W', 'Q'},
    {'G', 'F', 'D', 'S', 'A'},
    {'V', 'C', 'X', 'Z', CAPS_SHIFT},
    {'6', '7', '8', '9', '0'},
    {'Y', 'U', 'I', 'O', 'P'},
    {'H', 'J', 'K', 'L', KEY_ENTER},
    {'B', 'N', 'M', SYMBOL_SHIFT, ' '}
  },
  {
    //1 = CAPS_SHIFT
    {PS2_LEFTARROW, '4', '3', '2', '1'},
    {'T', 'R', 'E', 'W', 'Q'},
    {'G', 'F', 'D', 'S', 'A'},
    {'V', 'C', 'X', 'Z', CAPS_SHIFT},
    {PS2_DOWNARROW, PS2_UPARROW, PS2_LEFTARROW, '9', PS2_BACKSPACE},
    {'Y', 'U', 'I', 'O', 'P'},
    {'H', 'J', 'K', 'L', KEY_ENTER},
    {'B', 'N', 'M', SYMBOL_SHIFT, KEY_BRK}
  },
  {
    //2 = SYMBOL_SHIFT
    {'%', '$', '#', '@', '!'},
    {'>', '<', 'E', 'W', 'Q'},
    {'}', '{', '\\', '|', '~'},
    {'/', '?', 'X', ':', CAPS_SHIFT},
    {'&', '\'', '(', ')', '_'},
    {'Y', 'U', 'I', 'O', 'P'},
    {'^', '-', '+', '=', KEY_ENTER},
    {'*', ',', '.', SYMBOL_SHIFT, KEY_BRK}
  },
  {
    //3 = CAPS_SHIFT and SYMBOL_SHIFT
    {'5', '4', '3', '2', '1'},
    {'T', 'R', 'E', 'W', 'Q'},
    {'G', 'F', 'D', 'S', 'A'},
    {'V', 'C', 'X', 'Z', CAPS_SHIFT},
    {'6', '7', '8', '9', '0'},
    {'Y', 'U', 'I', 'O', 'P'},
    {'H', 'J', 'K', 'L', KEY_ENTER},
    {'B', 'N', 'M', SYMBOL_SHIFT, ' '}
  }

};

 uint8_t CharBuffer = 0;

uint8_t getScanCode(uint8_t r, uint8_t c)
{
static uint8_t shift = 0B00000000;
  //    sh &= 0B00000011; // выдаляем биты шифтов
  uint8_t sh = 0;
  uint8_t ret = charMap[sh][r][c];

  if (ret == CAPS_SHIFT)
  {
    shift ^= 0B00000001; // инвертируем CAPS_SHIFT
    ret = 0;
  }
  else if (ret == SYMBOL_SHIFT)
  {
    shift ^= 0B00000010; // инвертируем SUMBOL_SHIFT
    ret = 0;
  }
  else if (ret!=0){ //нажато что-то, но не шифт
    sh = shift & 0B00000011; //выделим биты шифтов - они дадут номер нужной таблицы символов
    ret = charMap[sh][r][c];
    shift &= 0B11111100; //обнулим биты шифтов
  }
  if (ret == CharBuffer)ret = 0;
  return ret;
}

// The ISR for the interrupt
ISR (TIMER2_COMPA_vect)
{
static uint8_t row = 255;
static uint8_t col = 255;
static uint8_t oldRow = 255;
static uint8_t oldCol = 255;
static uint8_t keyPressed = 0;
static uint8_t keyReleased = 255;

  cli();
  // Обработчик прерывания таймера 2
  col = 255;
  row = 255;
  for (uint8_t i = 0; i < COLS; i++)
  {
    PORTB &= ~(1 << i);
    delay (10);
    for (uint8_t j = 0; j < ROWS / 2; j++)
    {
      if (!(PINC & (1 << j)))
      {
        row = j; col = i;
        keyPressed = 1;
      }
      else if (!(PIND & (1 << (j + 2))))
      {
        row = j + 4; col = i;
        keyPressed = 1;
      }
    }
    PORTB |= (1 << i);
    delay(10);
  }
  if (keyPressed != 0 )
  {
    oldRow = row;
    oldCol = col;
    keyReleased = 1; //не отпущена
    keyPressed = 0; // готовы к следующему циклу чтения
  }
  else if (keyReleased = 1) // клавиша не нажата но была нажата на предыдущем цикле
  {
    keyReleased = 255; //для следующего нажатия
    CharBuffer = getScanCode(oldRow, oldCol);
    oldRow = 255; row = 255; //сбросим счетчики строки и столбца и промежуточное хранилище
    oldCol = 255; col = 255;

  }
  sei();
}


int matrixKB::available()
{
  if (CharBuffer) return true;
  return false;
}

int matrixKB::read()
{
  uint8_t result;
  result = CharBuffer;
  if (result)
  {
    CharBuffer = 0;
  }
  if (!result) return -1;
  return result;
}


matrixKB::matrixKB()
{
  // nothing to do here, begin() does it all
}

void matrixKB::begin()
{
  cli();
  DDRB  = 0b00011111;
  PORTB = 0b00000000;
//не включаем подтягивающие резисторы на входы
  DDRC  = 0b00000000;
  PORTC = 0b00000000; //0x0F;
  DDRD  = 0b00000000;
  PORTD = 0b00000000; 
  delay(10);

  TCCR2A = (1 << WGM21);  // Режим CTC (сброс по совпадению)
  // TCCR2B = (1<<CS20);                     // Тактирование от CLK.
  // TCCR2B = (1<<CS21);                     // CLK/8
  // TCCR2B = (1<<CS20)|(1<<CS21);           // CLK/32
  // TCCR2B = (1<<CS22);                     // CLK/64
  // TCCR2B = (1<<CS20)|(1<<CS22);           // CLK/128
  // TCCR2B = (1<<CS21)|(1<<CS22);           // CLK/256
  TCCR2B = (1 << CS20) | (1 << CS21) | (1 << CS22); // CLK/1024

  OCR2A = 255;            // Верхняя граница счета. Диапазон от 0 до 255.
  // Частота прерываний будет = Fclk/(N*(1+OCR2A))
  // где N - коэф. предделителя (1, 8, 32, 64, 128, 256 или 1024)
  TIMSK2 = (1 << OCIE2A); // Разрешить прерывание по совпадению
  sei();              // разрешаем прерывания (запрещаем: cli(); )
}

       int matrixKB::peek(){return 0;};
       void matrixKB::flush(){};
       size_t matrixKB::write(uint8_t){return 0;};



