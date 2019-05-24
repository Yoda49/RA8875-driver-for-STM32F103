#include "RA8875.h"
#include "stm32f10x.h"
#include "stm32_sba.h"

unsigned int _width = 800;
unsigned int _height = 480;
unsigned char _textScale;

#define CS_LCD_HIGH GPIOA->BSRR = 1 << 4
#define CS_LCD_LOW  GPIOA->BRR  = 1 << 4

#define BLUE_LED_OFF  GPIOC->BSRR = 1 << 13
#define BLUE_LED_ON GPIOC->BRR  = 1 << 13

#define DISPLAY_BUSY_STATUS GPIOA->IDR & 1 << 3



// ======================================================================================
// HARDWARE DELAY SECTION
// ======================================================================================
#define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
#define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC

void delay_ms(unsigned int ms)
{
	unsigned int ms_count_tick =  ms * (SystemCoreClock/1000);
	SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT_CYCCNT  = 0;
	DWT_CONTROL|= DWT_CTRL_CYCCNTENA_Msk; 
	while (DWT_CYCCNT < ms_count_tick);
	DWT_CONTROL &= ~DWT_CTRL_CYCCNTENA_Msk;
}


// ======================================================================================
// display init
// ======================================================================================
void ra8875_init (void)
{
	
  unsigned char pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
  unsigned char  hsync_start    = 32;
  unsigned char  hsync_pw       = 96;
  unsigned char  hsync_finetune = 0;
  unsigned char  hsync_nondisp  = 26;
  unsigned char  vsync_pw       = 2;
  unsigned int   vsync_nondisp  = 32;
  unsigned int   vsync_start    = 23;

	CONF_A (3, CNF_TRISTATE_IN, IN); // DISPLAY BUSY
	
	// SPI INIT
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // enable spi module
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // port a enable
	
	CONF_A (7, CNF_SPECIAL_OUT, OUT50); // MOSI SPI1
	CONF_A (6, CNF_TRISTATE_IN, IN);    // MISO SPI1
	CONF_A (5, CNF_SPECIAL_OUT, OUT50); // SCK SPI1
	CONF_A (4, CNF_DIGITAL_OUT, OUT50); // NSS SPI1
	
	SPI1->CR1 &= ~SPI_CR1_SPE; // SPI OFF
	SPI1->CR1 |= SPI_CR1_BR;   // low speed at start
	SPI1->CR1 |= SPI_CR1_CPOL;      // Polarity cls signal CPOL = 1;
  SPI1->CR1 |= SPI_CR1_CPHA;      // Phase cls signal    CPHA = 1;	
	//SPI1->CR1 &= ~SPI_CR1_DFF;      // 8 bit data
  SPI1->CR1 |= SPI_CR1_SSM;       // Soft NSS
  SPI1->CR1 |= SPI_CR1_SSI;       // Аналогично состоянию, когда на входе NSS высокий уровень
  SPI1->CR2 |= SPI_CR2_SSOE;      // Вывод NSS - выход управления slave select
	//SPI1->CR2 |= SPI_CR2_TXEIE;			// enable TX irq (if tx buffer is free)
  SPI1->CR1 |= SPI_CR1_MSTR;      // Master mode
  SPI1->CR1 |= SPI_CR1_SPE;       // SPI 1 on
	
	
	// RA8875 PLL CONFIGURE
  writeReg (RA8875_PLLC1, 11);
  delay_ms (100);
  writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
  delay_ms (100);
  writeReg (RA8875_SYSR, RA8875_SYSR_8BPP | RA8875_SYSR_MCU8);
  delay_ms (100);
  writeReg(RA8875_PCSR, pixclk);
  delay_ms (100);
  
	SPI1->CR1 &= ~SPI_CR1_SPE;
	SPI1->CR1 &= ~(SPI_CR1_BR_2 | SPI_CR1_BR_1); 
	SPI1->CR1 |= SPI_CR1_SPE;
	
	

  // Horizontal settings registers
  writeReg(RA8875_HDWR, 0x63); // (99 + 1) * 8 = 800 width
  writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
  writeReg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2)/8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
  writeReg(RA8875_HSTR, hsync_start / 8 - 1);                         // Hsync start: (HSTR + 1)*8 
  writeReg(RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw/8 - 1));        // HSync pulse width = (HPWR+1) * 8
  
  // Vertical settings registers
  writeReg(RA8875_VDHR0, 0xDF);  // 1DF = 479 height
  writeReg(RA8875_VDHR1, 0x01);
  writeReg(RA8875_VNDR0, vsync_nondisp-1);                          // V non-display period = VNDR + 1
  writeReg(RA8875_VNDR1, vsync_nondisp >> 8);
  writeReg(RA8875_VSTR0, vsync_start-1);                            // Vsync start position = VSTR + 1
  writeReg(RA8875_VSTR1, vsync_start >> 8);
  writeReg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1
  
  // Set active window X
  writeReg(RA8875_HSAW0, 0);                                        // horizontal start point
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F); // 31F = 799
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);                                        // vertical start point
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);           // horizontal end point
  writeReg(RA8875_VEAW1, 0x01);
  
  // ToDo: Setup touch panel?
  
  // Clear the entire window
  writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
  delay_ms (500); 
}


// ======================================================================================
// COLOR CONVERT RGB (3 byte) to RGB (1 byte)
// ======================================================================================
unsigned char rgb24_to_8 (unsigned char red, unsigned char green, unsigned char blue)
{
	red = red / 32;
	green = green / 32;
	blue = blue / 64;
	
	return (unsigned char)((red << 5) | (green << 2) | blue);
}


// ======================================================================================
// Software reset
// ======================================================================================
void ra8875_soft_reset(void)
{
  writeCommand(RA8875_PWRR);
  writeData(RA8875_PWRR_SOFTRESET);
  writeData(RA8875_PWRR_NORMAL);
  delay_ms(10);
}

// ======================================================================================
// Sets display on / off
// ======================================================================================
void ra8875_display_on (char on) 
{
	if (on) writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON); else writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}

// ======================================================================================
// Sets sleep
// ======================================================================================
void ra8875_sleep (char sleep) 
{
	if (sleep) writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF | RA8875_PWRR_SLEEP); else writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF);
}




// ======================================================================================
// Sets the current X/Y position on the display before drawing
//    
// x[in] The 0-based x location
// y[in] The 0-base y location
// ======================================================================================
void ra8875_set_xy_position (unsigned int x, unsigned int y)
{
  writeReg (RA8875_CURH0, (unsigned char)x);
  writeReg (RA8875_CURH1, (unsigned char)(x >> 8));
  writeReg (RA8875_CURV0, (unsigned char)y);
  writeReg (RA8875_CURV1, (unsigned char)(y >> 8));
}


// ======================================================================================
// HW accelerated function to push a chunk of raw pixel data
//      
// num[in] The number of pixels to push
// p[in]   The pixel color to use
// ======================================================================================
void ra8875_draw_image (unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char * array, unsigned long count)
{
	unsigned long i = 0;
	  // Set active window X
  writeReg(RA8875_HSAW0, (unsigned char)x1);
  writeReg(RA8875_HSAW1, (unsigned char)(x1 >> 8));
  writeReg(RA8875_HEAW0, (unsigned char)x2);
  writeReg(RA8875_HEAW1, (unsigned char)(x2 >> 8));
  
  // Set active window Y
  writeReg(RA8875_VSAW0, (unsigned char)y1);
  writeReg(RA8875_VSAW1, (unsigned char)(y1 >> 8));  
  writeReg(RA8875_VEAW0, (unsigned char)y2);
  writeReg(RA8875_VEAW1, (unsigned char)(y2 >> 8));
	
  writeReg (RA8875_CURH0, (unsigned char)x1);
  writeReg (RA8875_CURH1, (unsigned char)(x1 >> 8));
  writeReg (RA8875_CURV0, (unsigned char)y1);
  writeReg (RA8875_CURV1, (unsigned char)(y1 >> 8)); 
	
	CS_LCD_LOW;
	__nop ();
	
	SPI1->DR = RA8875_CMDWRITE;
	while (SPI1->SR & SPI_SR_BSY) {}
	SPI1->DR = RA8875_MRWC;
	while (SPI1->SR & SPI_SR_BSY) {}
		
	__nop ();
	CS_LCD_HIGH;
	__nop ();
	CS_LCD_LOW;
	__nop ();
	
	SPI1->DR = RA8875_DATAWRITE;
	while (SPI1->SR & SPI_SR_BSY) {}
		
  for (i = 0; i < count; i++)
	{
		SPI1->DR = array[i];
		while (SPI1->SR & SPI_SR_BSY) {}
  }

	__nop ();
	CS_LCD_HIGH;
	__nop ();
}





// ======================================================================================
// DRAW PIXEL
// ======================================================================================
void ra8875_draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{
  writeReg (RA8875_CURH0, x);
  writeReg (RA8875_CURH1, x >> 8);
  writeReg (RA8875_CURV0, y);
  writeReg (RA8875_CURV1, y >> 8);  
	writeReg (RA8875_MRWC, color);
}


// ======================================================================================
// Draws a HW accelerated line on the display
// ======================================================================================
void ra8875_draw_line (unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color)
{
  // Set active window X
  writeReg(RA8875_HSAW0, 0);
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F);
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);
  writeReg(RA8875_VEAW1, 0x01);
	
	writeReg (RA8875_CURH0, 0);
	writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
  /* Set X */
  writeReg(0x91, x0);
  writeReg(0x92, x0 >> 8);
  
  /* Set Y */
  writeReg(0x93, y0);
  writeReg(0x94, y0 >> 8);
  
  /* Set X1 */
  writeReg(0x95, x1);
  writeReg(0x96, x1 >> 8);

  /* Set Y1 */
  writeReg(0x97, y1);
  writeReg(0x98, y1 >> 8);
  
  /* Set Color */
  writeReg(RA8875_FGCR0, (color & 0xE0) >> 5); // red bits 7,6,5
  writeReg(RA8875_FGCR1, (color & 0x1C) >> 2);  // green bits 4,3,2
  writeReg(RA8875_FGCR2,  color & 0x03); // blue bits 1,0

  /* Draw! */
  writeReg(RA8875_DCR, 0x80);
  
	while (!(DISPLAY_BUSY_STATUS)) {}
}








// ======================================================================================
// Draws a HW accelerated circle on the display
//
// x[in]     The 0-based x location of the center of the circle
// y[in]     The 0-based y location of the center of the circle
// w[in]     The circle's radius
// color[in] The RGB565 color to use when drawing the pixel
// filled    True of false
// ======================================================================================
void ra8875_circle_draw(unsigned int x0, unsigned int y0, unsigned int r, unsigned char color, char filled)
{
  // Set active window X
  writeReg(RA8875_HSAW0, 0);
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F);
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);
  writeReg(RA8875_VEAW1, 0x01);
	
	writeReg (RA8875_CURH0, 0);
	writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
	
  /* Set X */
  writeReg (0x99, x0);
  writeReg (0x9a, x0 >> 8);
	
  /* Set Y */
  writeReg(0x9b, y0);
  writeReg(0x9c, y0 >> 8);	   
	
  /* Set Radius */
  writeReg(0x9d, r);
	
  /* Set Color */
  writeReg(RA8875_FGCR0, (color & 0xE0) >> 5); // red bits 7,6,5
  writeReg(RA8875_FGCR1, (color & 0x1C) >> 2);  // green bits 4,3,2
  writeReg(RA8875_FGCR2,  color & 0x03); // blue bits 1,0
	
  /* Draw! */
  if (filled) writeReg(RA8875_DCR, RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL); else writeReg(RA8875_DCR, RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);

	while (!(DISPLAY_BUSY_STATUS)) {}
}







// ======================================================================================
// Draws a HW accelerated circle square on the display
//
// x[in]     The 0-based x location of the center of the circle
// y[in]     The 0-based y location of the center of the circle
// w[in]     The circle's radius
// color[in] The RGB565 color to use when drawing the pixel
// filled    True of false
// ======================================================================================
void ra8875_circle_sqr_draw (unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1,unsigned char r, unsigned char color, char filled)
{
    // Set active window X
    writeReg(RA8875_HSAW0, 0);
    writeReg(RA8875_HSAW1, 0);
    writeReg(RA8875_HEAW0, 0x1F);
    writeReg(RA8875_HEAW1, 0x03);
  
    // Set active window Y
    writeReg(RA8875_VSAW0, 0);
    writeReg(RA8875_VSAW1, 0);  
    writeReg(RA8875_VEAW0, 0xDF);
    writeReg(RA8875_VEAW1, 0x01);
	
    writeReg (RA8875_CURH0, 0);
    writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
    /* Set X */
    writeReg(0x91, (unsigned char)x0);
    writeReg(0x92, (unsigned char)(x0 >> 8));

    /* Set Y */
    writeReg(0x93, (unsigned char)y0);
    writeReg(0x94, (unsigned char)(y0 >> 8));	   

    /* Set X1 */
    writeReg(0x95, (unsigned char)x1);
    writeReg(0x96, (unsigned char)(x1 >> 8));

    /* Set Y1 */
    writeReg(0x97, (unsigned char)y1);
    writeReg(0x98, (unsigned char)(y1 >> 8));   

    /* Set Radius */
    writeReg(0xA1, (unsigned char)r);
    writeReg(0xA2, (unsigned char)(r >> 8));   
    writeReg(0xA3, (unsigned char)r);
    writeReg(0xA4, (unsigned char)(r >> 8));   

    /* Set Color */
    writeReg(RA8875_FGCR0, (color & 0xE0) >> 5); // red bits 7,6,5
    writeReg(RA8875_FGCR1, (color & 0x1C) >> 2);  // green bits 4,3,2
    writeReg(RA8875_FGCR2,  color & 0x03); // blue bits 1,0

    /* Draw! */
    if (filled) writeReg(0xA0, 0xE0); else writeReg(0xA0, 0xA0);

    while (!(DISPLAY_BUSY_STATUS)) {}
}

// ======================================================================================
//      Draws a HW accelerated rectangle on the display
//
//      x[in]     The 0-based x location of the top-right corner
//      y[in]     The 0-based y location of the top-right corner
//      w[in]     The rectangle width
//      h[in]     The rectangle height
//      color[in] The RGB565 color to use when drawing the pixel
//			filled    True or false
// ======================================================================================
void ra8875_rectangle_draw(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char color, char filled)
{
  // Set active window X
  writeReg(RA8875_HSAW0, 0);
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F);
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);
  writeReg(RA8875_VEAW1, 0x01);
	
	writeReg (RA8875_CURH0, 0);
	writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
  /* Set X */
  writeReg(0x91, (unsigned char)x0);
  writeReg(0x92, (unsigned char)(x0 >> 8));
  
  /* Set Y */
  writeReg(0x93, (unsigned char)y0);
  writeReg(0x94, (unsigned char)(y0 >> 8));	   
  
  /* Set X1 */
  writeReg(0x95, (unsigned char)x1);
  writeReg(0x96, (unsigned char)(x1 >> 8));

  
  /* Set Y1 */
  writeReg(0x97, (unsigned char)y1);
  writeReg(0x98, (unsigned char)(y1 >> 8));

  /* Set Color */
  writeReg(0x63, (color & 0xE0) >> 5); // red bits 7,6,5
  writeReg(0x64, (color & 0x1C) >> 2);  // green bits 4,3,2
  writeReg(0x65,  color & 0x03); // blue bits 1,0

  /* Draw! */
  if (filled) writeReg(RA8875_DCR, 0xB0); else  writeReg(RA8875_DCR, 0x90);
  
	while (!(DISPLAY_BUSY_STATUS)) {}
}



// ======================================================================================
//      Draws a HW accelerated triangle on the display
//
//      x0[in]    The 0-based x location of point 0 on the triangle
//      0[in]     The 0-based y location of point 0 on the triangle
//      x1[in]    The 0-based x location of point 1 on the triangle
//      y1[in]    The 0-based y location of point 1 on the triangle
//      x2[in]    The 0-based x location of point 2 on the triangle
//      y2[in]    The 0-based y location of point 2 on the triangle
//      color[in] The RGB565 color to use when drawing the pixel
//			filled    True or false
// ======================================================================================
void ra8875_triangle_draw (unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color, char filled)
{
  // Set active window X
  writeReg(RA8875_HSAW0, 0);
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F);
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);
  writeReg(RA8875_VEAW1, 0x01);
	
	writeReg (RA8875_CURH0, 0);
	writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
  /* Set Point 0 */
  writeReg(0x91, x0);
  writeReg(0x92, x0 >> 8);
  writeReg(0x93, y0);
  writeReg(0x94, y0 >> 8);

  /* Set Point 1 */
  writeReg(0x95, x1);
  writeReg(0x96, x1 >> 8);
  writeReg(0x97, y1);
  writeReg(0x98, y1 >> 8);

  /* Set Point 2 */
  writeReg(0xA9, x2);
  writeReg(0xAA, x2 >> 8);
  writeReg(0xAB, y2);
  writeReg(0xAC, y2 >> 8);
  
  /* Set Color */
  writeReg(0x63, (color & 0xE0) >> 5); // red bits 7,6,5
  writeReg(0x64, (color & 0x1C) >> 2);  // green bits 4,3,2
  writeReg(0x65,  color & 0x03); // blue bits 1,0
  
  /* Draw! */
  if (filled) writeReg(RA8875_DCR, 0xA1); else writeReg(RA8875_DCR, 0x81);
  
  /* Wait for the command to finish */
  while (!(DISPLAY_BUSY_STATUS)) {}
}



// ======================================================================================
//      Draws a HW accelerated ellipse on the display
//
//      xCenter[in]   The 0-based x location of the ellipse's center
//      yCenter[in]   The 0-based y location of the ellipse's center
//      longAxis[in]  The size in pixels of the ellipse's long axis
//      shortAxis[in] The size in pixels of the ellipse's short axis
//      color[in]     The RGB565 color to use when drawing the pixel
//			filled    		True or false
// ======================================================================================
void ra8875_ellipse_draw (unsigned int xCenter, unsigned int yCenter, unsigned int longAxis, unsigned int shortAxis, unsigned char color, char filled)
{
  // Set active window X
  writeReg(RA8875_HSAW0, 0);
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F);
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);
  writeReg(RA8875_VEAW1, 0x01);
	
	writeReg (RA8875_CURH0, 0);
	writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
  /* Set Center Point */
  writeReg(0xA5, xCenter);
  writeReg(0xA6, xCenter >> 8);
  writeReg(0xA7, yCenter);
  writeReg(0xA8, yCenter >> 8);

  /* Set Long and Short Axis */
  writeReg(0xA1, longAxis);
  writeReg(0xA2, longAxis >> 8);
  writeReg(0xA3, shortAxis);
  writeReg(0xA4, shortAxis >> 8);
  
  /* Set Color */
  writeReg(0x63, (color & 0xE0) >> 5); // red bits 7,6,5
  writeReg(0x64, (color & 0x1C) >> 2);  // green bits 4,3,2
  writeReg(0x65,  color & 0x03); // blue bits 1,0
  
  /* Draw! */
  if (filled) writeReg(0xA0, 0xC0); else writeReg(0xA0, 0x80);
  
  /* Wait for the command to finish */
  while (!(DISPLAY_BUSY_STATUS)) {}
}

// ======================================================================================
//      Draws a HW accelerated filled curve on the display
//
//      xCenter[in]   The 0-based x location of the ellipse's center
//      yCenter[in]   The 0-based y location of the ellipse's center
//      longAxis[in]  The size in pixels of the ellipse's long axis
//      shortAxis[in] The size in pixels of the ellipse's short axis
//      curvePart[in] The corner to draw, where in clock-wise motion:
//                    0 = 180-270°
//                    1 = 270-0°
//                    2 = 0-90°
//                    3 = 90-180°
//      color[in]     The RGB565 color to use when drawing the pixel
//			filled    		True or false
// ======================================================================================
void ra8875_curve_draw (unsigned int xCenter, unsigned int yCenter, unsigned int longAxis, unsigned int shortAxis, unsigned char curvePart, unsigned char color, char filled)
{
  // Set active window X
  writeReg(RA8875_HSAW0, 0);
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, 0x1F);
  writeReg(RA8875_HEAW1, 0x03);
  
  // Set active window Y
  writeReg(RA8875_VSAW0, 0);
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, 0xDF);
  writeReg(RA8875_VEAW1, 0x01);
	
	writeReg (RA8875_CURH0, 0);
	writeReg (RA8875_CURH1, 0);
	writeReg (RA8875_CURV0, 0);
	writeReg (RA8875_CURV1, 0); 
	
  /* Set Center Point */
  writeReg(0xA5, xCenter);
  writeReg(0xA6, xCenter >> 8);
  writeReg(0xA7, yCenter);
  writeReg(0xA8, yCenter >> 8);

  /* Set Long and Short Axis */
  writeReg(0xA1, longAxis);
  writeReg(0xA2, longAxis >> 8);
  writeReg(0xA3, shortAxis);
  writeReg(0xA4, shortAxis >> 8);
  
  /* Set Color */
  writeReg(0x63, (color & 0xE0) >> 5); // red bits 7,6,5
  writeReg(0x64, (color & 0x1C) >> 2);  // green bits 4,3,2
  writeReg(0x65,  color & 0x03); // blue bits 1,0

  /* Draw! */
  if (filled) writeReg(0xA0, 0xD0 | (curvePart & 0x03)); else writeReg(0xA0, 0x90 | (curvePart & 0x03));
  
  /* Wait for the command to finish */
  while (!(DISPLAY_BUSY_STATUS)) {}
}

// ======================================================================================
// GPIOX
// ======================================================================================
void ra8875_gpiox(char on)
{
  if (on) writeReg(RA8875_GPIOX, 1); else writeReg(RA8875_GPIOX, 0);
}

// ======================================================================================
// PWM1 OUT
// ======================================================================================
void ra8875_pwm1_out(unsigned char p)
{
  writeReg(RA8875_P1DCR, p);
	
	delay_ms (100);
}

// ======================================================================================
// PWM2 OUT
// ======================================================================================
void ra8875_pwm2_out(unsigned char p)
{
  writeReg(RA8875_P2DCR, p);
}

// ======================================================================================
// PWM1 config
// ======================================================================================
void ra8875_pwm1_config (char on, unsigned char clock)
{
  if (on) writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF)); else writeReg(RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
}

// ======================================================================================
// PWM2 config
// ======================================================================================
void ra8875_pwm2_config (char on, unsigned char clock)
{
  if (on) writeReg(RA8875_P2CR, RA8875_P2CR_ENABLE | (clock & 0xF)); else writeReg(RA8875_P2CR, RA8875_P2CR_DISABLE | (clock & 0xF));
}

































// ======================================================================================
// LOW LEVEL FUNCTIONS
// ======================================================================================

void writeReg (unsigned char reg, unsigned char val) 
{
	CS_LCD_LOW;
	__nop ();
	
	SPI1->DR = RA8875_CMDWRITE;
	while (SPI1->SR & SPI_SR_BSY) {}
	SPI1->DR = reg;
	while (SPI1->SR & SPI_SR_BSY) {}
		
	__nop ();
	CS_LCD_HIGH;
	__nop ();
	CS_LCD_LOW;		
	__nop ();
		
	SPI1->DR = RA8875_DATAWRITE;
	while (SPI1->SR & SPI_SR_BSY) {}
	SPI1->DR = val;
	while (SPI1->SR & SPI_SR_BSY) {}
		
	__nop ();
	CS_LCD_HIGH;
	__nop ();
}



void writeData (unsigned char d) 
{
	CS_LCD_LOW;
	__nop ();
	
	SPI1->DR = RA8875_DATAWRITE;
	while (SPI1->SR & SPI_SR_BSY) {}
	SPI1->DR = d;
	while (SPI1->SR & SPI_SR_BSY) {}
		
	__nop ();
	CS_LCD_HIGH;
	__nop ();
}


void writeCommand (unsigned char d) 
{
	CS_LCD_LOW;
	__nop ();
	
	SPI1->DR = RA8875_CMDWRITE;
	while (SPI1->SR & SPI_SR_BSY) {}
	SPI1->DR = d;
	while (SPI1->SR & SPI_SR_BSY) {}
		
	__nop ();
	CS_LCD_HIGH;
	__nop ();
}


