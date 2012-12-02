

/* Copyright (c) 2009, Peter Barrett  
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
**  
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.  
*/

// LCD Driver
//

#include "Platform.h"
#include "Board.h"
#include "LCD.h"

uint16_t _color;
void WriteLcdRegAddress(uint16_t addr);
void WriteLcdReg(uint16_t addr, uint16_t data);
uint16_t ReadLcdReg(uint16_t addr);
void Blit(uint16_t count, byte a, byte b);

//#define LCD_SSD1297
//#define LCD_HX8347
//#define LCD_ILI9325
#define LCD_SSD1289

#ifdef LCD_SSD1297
#include "LCD_SSD1297.h"    // Solomon Systech SSD1297 (i.e. SMPK8858)
#endif

#ifdef LCD_HX8347
#include "LCD_HX8347.h"
#endif

#ifdef LCD_SSD1289
#include "LCD_SSD1289.h"
#endif

#ifdef LCD_ILI9325
#include "LCD_ILI9325.h"    // Default is ILITEK LCD ILI9325      (i.e. SMPK8858B)
#endif

void WriteLcdRegAddress(uint16_t addr)
{
    DATAOUT;
    DATALPORT = (uint8_t)addr; 
    DATAHPORT = 0; // (uint8_t)(addr >> 8); // always zero
    RS0;  // A0 address
    WR0;
    CS0;  // Select chip
    CS1;  // Select chip
    WR1;
    RS1;  // Written address
}

void WriteLcdReg(uint16_t addr, uint16_t data)
{
    WriteLcdRegAddress(addr);
    DATALPORT = (uint8_t)data;
    DATAHPORT = (uint8_t)(data >> 8);
    WR0;
    CS0;
    CS1;
    WR1;
    DATAIN;  // Release Data bus
}
            
uint16_t ReadLcdReg(uint16_t addr)
{
    uint16_t result;
    WriteLcdRegAddress(addr);
    DATAIN;  // Release Data bus
    RD0;
    RD0;
    RD0;
    result = DATAHPIN;
    result = (result << 8) | DATALPIN;
    RD1;
    CS1;
    return result;
}

int LCD_::Width()
{
    return 240;
}

int LCD_::Height()
{
    return 320;
}

void LCD_::Blit(const byte* d, u32 count)
{
    DATAOUT;
    byte w1 = CONTROLPORT;
    byte w0 = w1 & ~(1 << WR);

    byte slow = count & 0x03;
    if (slow)
    {
        do {
            DATAHPORT = d[0];
            DATALPORT = d[1];
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            d+=2;
        } while (--slow);
    }
    
    // x4 unrolled
    count >>= 2;
    if (count)
    {
		int c = count;
        byte w1 = CONTROLPORT;
        byte w0 = w1 & ~(1 << WR);
        do {
            DATAHPORT = d[0];
            DATALPORT = d[1];
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            DATAHPORT = d[2];
            DATALPORT = d[3];
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            DATAHPORT = d[4];
            DATALPORT = d[5];
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            DATAHPORT = d[6];
            DATALPORT = d[7];
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            d += 8;
        } while (--c);
    }
}

void LCD_::BlitIndexed(const byte* d, const byte* palette, u32 count32)
{
    byte w1 = CONTROLPORT;
    byte w0 = w1 & ~(1 << WR);
	int count = count32;	// Will never overflow

    // Looks fussy but it attempts to be fast
    while (count)
    {
        byte c = 255;
        if (count < c)
            c = count;
        byte p = d[0];
        byte i = 0;
        do
        {
            const byte* b = palette + p*2;
            DATAHPORT = b[0];
            DATALPORT = b[1];
            byte n;
            do
            {
                CONTROLPORT = w0;
                CONTROLPORT = w1;
                n = d[++i];
            } while (p == n && i < c);
            p = n;
        } while (i < c);
        count -= c;
        d += c;
    }
}

void LCD_::Fill(int color, u32 count32)
{
    u8 a = color >> 8;
    u8 b = color;
    DATAOUT;
    DATAHPORT = a;
    DATALPORT = b;

    do {
        WR0;
        CS0;
        CS1;
        WR1;
    } while (--count32);
    
/*
    byte slow = count32 & 0x07;
    if (slow)
    {
        do {
            WR0;
            WR1;
        } while (--slow);
    }
    
    // x8 unrolled
	int count = count32 >> 3;
    if (count)
    {
        byte w1 = CONTROLPORT;
        byte w0 = w1 & ~(1 << WR);
        do {
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            CONTROLPORT = w0;
            CONTROLPORT = w1;

            CONTROLPORT = w0;
            CONTROLPORT = w1;
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            CONTROLPORT = w0;
            CONTROLPORT = w1;
            CONTROLPORT = w0;
            CONTROLPORT = w1;
        } while (--count);
    }
*/
}
