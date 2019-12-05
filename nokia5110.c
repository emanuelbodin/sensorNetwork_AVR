/*
 * nokia5110.c
 *
 * Created: 2015-06-09 21:07:02
 *  Author: uwezi
 */ 

// NOKIA routines as external file...

#include <stdlib.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>
#include <string.h>


#include "nokia5110.h"

#ifndef F_CPU
#define F_CPU 20000000UL   // save to assume the fastest clock
#endif

#include <util/delay.h>


/*
** constants/macros   (thanks Peter Fleury)
*/
#define DDR(x) (*(&x - 1))      /* address of data direction register of port x */
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
/* on ATmega64/128 PINF is on port 0x00 and not 0x60 */
#define PIN(x) ( &PORTF==&(x) ? _SFR_IO8(0x00) : (*(&x - 2)) )
#else
#define PIN(x) (*(&x - 2))    /* address of input register of port x          */
#endif


//
// macros for the software SPI
//

#define SetRST     RST_PORT |= (1 << RST) 
#define ClearRST   RST_PORT &=~(1 << RST)
#define SetSCE     SCE_PORT |= (1 << SCE)
#define ClearSCE   SCE_PORT &=~(1 << SCE)
#define SetDC      DC_PORT  |= (1 << DC)
#define ClearDC    DC_PORT  &=~(1 << DC)
#define SetSD      SD_PORT  |= (1 << SD)
#define ClearSD    SD_PORT  &=~(1 << SD)
#define SetSCL     SCL_PORT |= (1 << SCL)
#define ClearSCL   SCL_PORT &=~(1 << SCL)  


//  6x8 font
//    LSB is top
//    MSB is bottom
//
static const uint8_t smallFont[][6] PROGMEM =
#include "font_6x8_iso8859_1.h"

//
//  define a local copy of the display memory
//
uint8_t framebuffer[LCDSIZEX*LCDSIZEY/8];

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeCommand
  Description  :  Sends command to display controller.
  Argument(s)  :  command -> command to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeCommand (uint8_t command )
{
  uint8_t i;
  // the bit-banging routines may not be interrupted!
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ClearSCE;       //enable LCD
    ClearDC;        // set LCD into command mode
    ClearSCL;
    for (i=0; i<8; i++)
    {
      if (command & 0b10000000)
      {
        SetSD;
      }
      else
      {
        ClearSD;
      }
      SetSCL;       // minimum 100 ns
      ClearSCL;     // minimum 100 ns
      command <<= 1;
    }
    SetSCE;         // disable LCD
  }  
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeData
  Description  :  Sends data to display controller.
  Argument(s)  :  data -> data to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeData (uint8_t data )
{
    uint8_t i;
    // the bit-banging routines may not be interrupted!
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      ClearSCE;       // enable LCD
      SetDC;          // set LCD in data mode
      ClearSCL;
#ifdef LCDROTATE
      for (i=0; i<8; i++)
      {
        if (data & 0b00000001)
        {
          SetSD;
        }
        else
        {
          ClearSD;
        }
        SetSCL;       // minimum 100 ns
        ClearSCL;     // minimum 100 ns
        data >>= 1;
      }
#else
      for (i=0; i<8; i++)
      {
        if (data & 0b10000000)
        {
          SetSD;
        }
        else
        {
          ClearSD;
        }
        SetSCL;       // minimum 100 ns
        ClearSCL;     // minimum 100 ns
        data <<= 1;
      }
#endif
      SetSCE;         // disable LCD
    }      
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_gotoXY
  Description  :  Sets cursor location to xy location corresponding to basic font LCDSIZE.
  Argument(s)  :  x - range: 0 to 84
                  y -> range: 0 to 5
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_gotoXY ( uint8_t x, uint8_t y )
{
    LCD_writeCommand (0x80 | x);   //column
    LCD_writeCommand (0x40 | y);   //row
}

void LCD_clearbuffer(void)
{
  memset(framebuffer, 0x00, LCDSIZEX*LCDSIZEY/8);
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_update
  Description  :  transfers the local copy to the display
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_update (void)
{
  uint16_t i;

  LCD_gotoXY(0,0);      // start with (0,0) position
  for(i=0; i<(LCDSIZEX*LCDSIZEY/8); i++)
  {
#ifdef LCDROTATE
    LCD_writeData(framebuffer[LCDSIZEX*LCDSIZEY/8-i-1]);
#else
    LCD_writeData(framebuffer[i]);
#endif
  }
  LCD_gotoXY(0,0);      // bring the XY position back to (0,0)
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_clear
  Description  :  Clears the display
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_clear ( void )
{
    LCD_clearbuffer();
    LCD_update();
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_init
  Description  :  LCD controller initialization.
  Argument(s)  :  contrast value VOP
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_init (uint8_t vop)
{

  DDR(SCE_PORT) |= (1 << SCE);
  DDR(RST_PORT) |= (1 << RST);
  DDR(SCL_PORT) |= (1 << SCL);
  DDR(DC_PORT)  |= (1 << DC);
  DDR(SD_PORT)  |= (1 << SD);
  DDR(BGLED_PORT)  |= (1 << BGLED);

  _delay_ms(100);

  // the bit-banging routines may not be interrupted!
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ClearSCE;                  // Enable LCD
    ClearRST;                  // reset LCD
    
    _delay_ms(100);
    SetRST;
    SetSCE;                    //disable LCD
  }  
  
  LCD_writeCommand( 0x21 );  // LCD Extended Commands.
  LCD_writeCommand( 0x80 | vop );  // Set LCD Vop (Contrast).
  LCD_writeCommand( 0x04 );  // Set Temp coefficent.
  LCD_writeCommand( 0x13 );  // LCD bias mode 1:48.
  LCD_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
  LCD_writeCommand( 0x0c );  // LCD in normal mode.

  LCD_clear();
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_setVop
  Description  :  Sets the contrast voltage
  Argument(s)  :  VOP 0 to 127
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_setVop(uint8_t vop)
{
    LCD_writeCommand( 0x21 );  // LCD Extended Commands.
    LCD_writeCommand( 0x80 | vop );  // Set LCD Vop (Contrast).
    LCD_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
    LCD_writeCommand( 0x0c );  // LCD in normal mode.
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_setpixel
  Description  :  Sets the pixel at xy location
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_setpixel(uint8_t x, uint8_t y)
{
  if ((x < LCDSIZEX) && (y < LCDSIZEY))
  {
    framebuffer[(uint16_t) x+LCDSIZEX*(y/8)] |= (1 << (y % 8));
  }
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_clearpixel
  Description  :  Clears the pixel at xy location
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_clearpixel(uint8_t x, uint8_t y)
{
  if ((x < LCDSIZEX) && (y < LCDSIZEY))
  {
    framebuffer[(uint16_t) x+LCDSIZEX*(y/8)] &= ~(1 << (y % 8));
  }
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_putchar
  Description  :  puts a single character onto LCD
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
                  ch - character
                  attr - attribute 0-normal, 1-inverse, 2-underline
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_putchar(uint8_t x0, uint8_t y0, char ch, uint8_t attr)
{
  uint8_t yd, ym, i, fontbyte;
  uint16_t m;
  yd = y0/8;
  ym = y0%8;
  for (i=0; i<6; i++)
  {
    fontbyte = pgm_read_byte(&smallFont[(uint8_t)ch][i]);
    switch (attr)
    {
      case  0:
          break;
      case  1: 
          fontbyte ^= 0xff;
          break;
      case  2: 
          fontbyte |= 0b10000000;
          break;
    }

    if ((x0+i)<LCDSIZEX)
    {
      m = (uint16_t) x0+i+LCDSIZEX*(yd);
      framebuffer[m] &= ~(0xff << ym);
      framebuffer[m] |= (fontbyte << ym);
      if ((y0<(LCDSIZEY-8)) && (ym != 0))
      {
        m = (uint16_t) x0+i+LCDSIZEX*(yd+1);
        framebuffer[m] &= ~(0xff >> (8-ym));
        framebuffer[m] |= (fontbyte >> (8-ym));
      }
    }
  }
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_print
  Description  :  prints a string
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
                  *ch - pointer t string
                  attr - attribute 0-normal, 1-inverse, 2-underline
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_print(uint8_t x, uint8_t y, char *ch,uint8_t attr)
{
  while (*ch)
  {
    LCD_putchar(x, y, *ch, attr);
    ch++;
    x += 6;
  }
}

void LCD_print_p(uint8_t x, uint8_t y, const char *ch,uint8_t attr)
{
  char c;
  while ((c = pgm_read_byte(ch)))
  {
    LCD_putchar(x, y, c, attr);
    ch++;
    x += 6;
  }
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_scroll
  Description  :  softscrolls the framebuffer
  Argument(s)  :  dy - range: 0 to +/-47
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_scroll(int8_t dy)
{
  int8_t y1;
  uint8_t  x, y, dy1, dy8, b1, b2;
  if (dy>0)
  {
    dy8 = dy/8;
    dy1 = dy%8;
    for (x=0; x<LCDSIZEX; x++)
    {
      for (y=0; y<(LCDSIZEY/8); y++)
      {
        y1=y+dy8;
        if (y1<(LCDSIZEY/8))
        {
          b1 = framebuffer[x + LCDSIZEX*y1];
        }
        else
        {
          b1=0;
        }
        if ((y1+1)<(LCDSIZEY/8))
        {
          b2 = framebuffer[x + LCDSIZEX*(y1+1)];
        }
        else
        {
          b2=0;
        }
        framebuffer[x + LCDSIZEX*(y)] = (b1 >> dy1) | (b2 << (8-dy1));
      }
    }
  } 
  else
  {
    dy8 = abs(dy)/8;
    dy1 = abs(dy)%8;
    for (x=0; x<LCDSIZEX; x++)
    {
      for (y=0; y<(LCDSIZEY/8); y++)
      {
        y1=(LCDSIZEY/8)-y-dy8-1;
        if (y1>=0)
        {
          b1 = framebuffer[x + LCDSIZEX*y1];
        }
        else
        {
          b1=0;
        }
        if ((y1-1)>=0)
        {
          b2 = framebuffer[x + LCDSIZEX*(y1-1)];
        }
        else
        {
          b2=0;
        }
        framebuffer[x + LCDSIZEX*((LCDSIZEY/8)-y-1)] = (b1 << dy1) | (b2 >> (8-dy1));
      }
    }
  }
}
