/*
 * nokia5110.h
 *
 * Created: 2015-06-09 21:07:15
 *  Author: uwezi
 */ 


#ifndef NOKIA5110_H_
#define NOKIA5110_H_

#include <avr/pgmspace.h>

//
// macros for the software SPI
//

#define RST        PC5
#define RST_PORT   PORTC

#define SCE        PC4
#define SCE_PORT   PORTC

#define SCL        PC1
#define SCL_PORT   PORTC

#define DC         PC3
#define DC_PORT    PORTC

#define SD         PC2
#define SD_PORT    PORTC

#define BGLED        PD0
#define BGLED_PORT   PORTD
#define BGLED_ON()   BGLED_PORT |= (1 << BGLED)
#define BGLED_OFF()  BGLED_PORT &= ~(1 << BGLED)

#define LCDROTATE

#define LCDSIZEX 84
#define LCDSIZEY 48
extern uint8_t framebuffer[LCDSIZEX*LCDSIZEY/8];

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeCommand
  Description  :  Sends command to display controller.
  Argument(s)  :  command -> command to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_writeCommand (uint8_t command );

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeData
  Description  :  Sends data to display controller.
  Argument(s)  :  data -> data to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_writeData (uint8_t data );

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_gotoXY
  Description  :  Sets cursor location to xy location corresponding to basic font size.
  Argument(s)  :  x - range: 0 to 84
                  y -> range: 0 to 5
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_gotoXY ( uint8_t x, uint8_t y );

extern void LCD_clearbuffer(void);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_update
  Description  :  transfers the local copy to the display
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_update (void);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_clear
  Description  :  Clears the display
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_clear ( void );

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_init
  Description  :  LCD controller initialization.
  Argument(s)  :  contrast value VOP
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_init (uint8_t vop);

extern void LCD_setVop(uint8_t vop);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_setpixel
  Description  :  Sets the pixel at xy location
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_setpixel(uint8_t x, uint8_t y);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_clearpixel
  Description  :  Clears the pixel at xy location
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_clearpixel(uint8_t x, uint8_t y);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_putchar
  Description  :  puts a single character onto LCD
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
                  ch - character
                  attr - attribute 0-normal, 1-inverse, 2-underline
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_putchar(uint8_t x0, uint8_t y0, char ch, uint8_t attr);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_print
  Description  :  prints a string
  Argument(s)  :  x - range: 0 to 83
                  y - range: 0 to 47
                  *ch - pointer t string
                  attr - attribute 0-normal, 1-inverse, 2-underline
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
extern void LCD_print(uint8_t x, uint8_t y, char *ch, uint8_t attr);

extern void LCD_print_p(uint8_t x, uint8_t y, const char *ch, uint8_t attr);

/**
 * @brief    Macro to automatically put a string constant into program memory
 */
#define LCD_print_P(x, y, __s, attr)       LCD_print_p(x, y, PSTR(__s), attr);

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_scroll
  Description  :  softscrolls the framebuffer
  Argument(s)  :  dy - range: 0 to +/-47
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_scroll(int8_t dy);

#endif /* NOKIA5110_H_ */