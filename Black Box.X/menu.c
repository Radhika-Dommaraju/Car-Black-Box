/*
 * File:   menu.c
 * Author: Radhika
 *
 * Created on 15 October, 2026, 4:55 PM
 */


#include <xc.h>
#include "menu.h"
#include "CLCD.h"
#include "ds1307.h"
#include "i2c.h"
#include "Matrix keypad.h"
#include "ext eeprom.h"
#include "adc.h"
#include "newmain.h"
#define MAX_EVENTS 10
#define EVENT_SIZE 12



void main_menu()
{
   
    if(menu == 0)
    {
    clcd_print("->VIEW LOG", LINE1(0));
    clcd_print("DOWNLOAD LOG", LINE2(0));
    }
    else if (menu == 1)
    {
    clcd_print("->DOWNLOAD LOG", LINE1(0));
    clcd_print("CLEAR LOG", LINE2(0));
    }
    else if (menu == 2)
    {
    clcd_print("->CLEAR LOG", LINE1(0));
    clcd_print("SET TIME", LINE2(0));
    }
    else if (menu == 3)    
    {
    clcd_print("->SET TIME", LINE1(0));
    clcd_print(" ", LINE2(0));
    }
}