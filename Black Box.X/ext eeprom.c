/*
 * File:   ext eeprom.c
 * Author: Radhika
 *
 * Created on 8 October, 2026, 1:46 PM
 */


#include <xc.h>

#define _XTAL_FREQ 20000000UL

#include "ext eeprom.h"
#include "i2c.h"

void write_ext_eeprom(unsigned char address,  unsigned char data)
{
    
	i2c_start();
	i2c_write(SLAVE_WRITE);
	i2c_write(address);
	i2c_write(data);
	i2c_stop();
    
    
    __delay_ms(10);
}


unsigned char read_ext_eeprom(unsigned char address)
{
    unsigned char data;

	i2c_start();
	i2c_write(SLAVE_WRITE);
	i2c_write(address);
	i2c_rep_start();
	i2c_write(SLAVE_READ);
	data = i2c_read();
	i2c_stop();

	return data;
  
}
