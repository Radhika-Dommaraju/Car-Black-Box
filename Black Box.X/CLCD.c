/*
 * File:   CLCD.c
 * Author: Radhika
 *
 * Created on 8 October, 2026, 12:53 PM
 */


#include <xc.h>
#include "clcd.h"

void clcd_write(unsigned char byte, unsigned char control_bit)
{
    CLCD_RS = control_bit;
    CLCD_RW = LO;

    PORT_DIR = OUTPUT;
    CLCD_PORT = byte;

    /* Enable pulse */
    CLCD_EN = HI;
    __delay_us(1);
    CLCD_EN = LO;

    /* Now read busy flag */
    PORT_DIR = INPUT;
    CLCD_RW = HI;
    CLCD_RS = INSTRUCTION_COMMAND;

    do
    {
        CLCD_EN = HI;
        __delay_us(1);

        CLCD_EN = LO;
        __delay_us(1);

    } while (CLCD_BUSY);

    CLCD_RW = LO;
    PORT_DIR = OUTPUT;
}

void init_clcd()
{
    TRISD = 0x00;
    TRISC = TRISC & 0xF8;

    CLCD_RW = LO;
    CLCD_EN = LO;

    __delay_ms(30);

    clcd_write(0x30, INSTRUCTION_COMMAND);
    __delay_ms(5);

    clcd_write(0x30, INSTRUCTION_COMMAND);
    __delay_us(150);

    clcd_write(0x30, INSTRUCTION_COMMAND);
    __delay_us(150);

    clcd_write(0x38, INSTRUCTION_COMMAND);   // 8-bit, 2-line
    clcd_write(0x01, INSTRUCTION_COMMAND);   // clear
    __delay_ms(2);

    clcd_write(0x0C, INSTRUCTION_COMMAND);   // display ON
}

void clcd_print(const unsigned char *data, unsigned char addr)
{
	clcd_write(addr, INSTRUCTION_COMMAND);
	while (*data != '\0')
	{
		clcd_write(*data++, DATA_COMMAND);
	}
}

void clcd_putch(const unsigned char data, unsigned char addr)
{
	clcd_write(addr, INSTRUCTION_COMMAND);
	clcd_write(data, DATA_COMMAND);
}
