/*
 * File:   newmain.c
 * Author: Radhika
 *
 * Created on 8 October, 2026, 12:53 PM
 */

#include <xc.h>
#include "CLCD.h"
#include "ds1307.h"
#include "i2c.h"
#include "Matrix keypad.h"
#include "ext eeprom.h"
#include "adc.h"
#include "newmain.h"
#include "UART.h"

#define MAX_EVENTS 10
#define EVENT_SIZE 12

unsigned char clock_reg[3];
unsigned char time[9];
unsigned int event_count= 0, key, ind = 0, speed, address = 0, menu = 0, index =0;
unsigned char speed_str[3], event[MAX_EVENTS][16];
unsigned short adc_value;
unsigned char flagm = 0, viewmode = 0, write_index = 0, clearmode = 0;


void init_config(void)
{
	init_clcd();
	init_i2c();
	init_ds1307();
    init_matrix_keypad();
    init_adc();
    init_uart();
}

static void get_time(void)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    /* Hours */
    time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
    time[1] = '0' + (clock_reg[0] & 0x0F);

    time[2] = ':';

    /* Minutes */
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);

    time[5] = ':';

    /* Seconds */
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);

    time[8] = '\0';
}


unsigned char gear[9][3] = { "ON", "GN", "G1", "G2", "G3", "G4", "G5", "GR", "C_"};

void store_event(void)
{
    unsigned int i;
    unsigned int address;

    address = write_index * EVENT_SIZE;

    for (i = 0; i < 8; i++)
    {
        write_ext_eeprom(address++, time[i]);
    }

    for (i = 0; i < 2; i++)
    {
        write_ext_eeprom(address++, gear[ind][i]);
    }

    for (i = 0; i < 2; i++)
    {
        write_ext_eeprom(address++, speed_str[i]);
    }

    // Move to the next EEPROM slot
    write_index++;

    if (write_index >= MAX_EVENTS)
    {
        write_index = 0;
    }

    // Keep the valid event count at a maximum of 10
    if (event_count < MAX_EVENTS)
    {
        event_count++;
    }
}



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

    
  void read_event(void)
  {
    unsigned int i, j;
    unsigned int read_address;
    unsigned int slot;

    for (i = 0; i < event_count; i++)
    {
        
        // write_index points to the oldest event.
        if (event_count < MAX_EVENTS)
        {
            slot = i;
        }
        else
        {
            slot = (write_index + i) % MAX_EVENTS;
        }

        read_address = slot * EVENT_SIZE;

        for (j = 0; j < 8; j++)
        {
            event[i][j] = read_ext_eeprom(read_address++);
        }

        event[i][8] = ' ';
        event[i][9] = read_ext_eeprom(read_address++);
        event[i][10] = read_ext_eeprom(read_address++);
        event[i][11] = ' ';
        event[i][12] = read_ext_eeprom(read_address++);
        event[i][13] = read_ext_eeprom(read_address++);
        event[i][14] = '\0';
    }
  }
  
  
void view_log(void)
{
    CLEAR_DISP_SCREEN;

    if (event_count == 0)
    {
        clcd_print("NO EVENTS", LINE1(0));
        return;
    }

    if (index >= event_count)
    {
        index = 0;
    }

    clcd_print(event[index], LINE1(0));

    if (index + 1 < event_count)
    {
        clcd_print(event[index + 1], LINE2(0));
    }
    else
    {
        clcd_print("                ", LINE2(0));
    }
}


void clear_log(void)
{
    CLEAR_DISP_SCREEN;

    event_count = 0;
    write_index = 0;
    index = 0;
    address = 0;

    clcd_print("LOG CLEARED", LINE1(0));

    for (unsigned long i = 0; i < 500; i++)
    {
        __delay_ms(10);
    }

    CLEAR_DISP_SCREEN;

}
//
//void download_log(void)
//{
//    clcd_print("Downloading....", LINE1(0));
//    
//    for (unsigned long j = 0; j < 500; j++)
//    {
//        __delay_ms(10);
//    }
//    
//    if (event_count == 0)
//    {
//        clcd_print("NO EVENTS", LINE2(0));
//        __delay_ms(1000);
//        return;
//    }
//    
//    read_event();
//    
//    
//}

void download_log(void)
{
    unsigned int i, j;

    CLEAR_DISP_SCREEN;
    clcd_print("DOWNLOADING...", LINE1(0));

    if (event_count == 0)
    {
        clcd_print("NO EVENTS", LINE2(0));
        __delay_ms(1000);
        return;
    }

    // Read events from external EEPROM
    read_event();

    // Send heading to Tera Term
    puts("\r\nCAR BLACK BOX LOG\r\n");
    puts("--------------------------------\r\n");
    puts("S.NO  TIME      GEAR  SPEED\r\n");
    puts("--------------------------------\r\n");

    for (i = 0; i < event_count; i++)
    {
        // Serial number
        putch('0' + i + 1);
        puts("     ");

        // Time (HH:MM:SS)
        for (j = 0; j < 8; j++)
        {
            putch(event[i][j]);
        }

        puts("   ");

        // Gear
        putch(event[i][9]);
        putch(event[i][10]);

        puts("    ");

        // Speed
        putch(event[i][12]);
        putch(event[i][13]);

        puts("\r\n");
    }

    puts("--------------------------------\r\n");
    puts("DOWNLOAD COMPLETE\r\n\r\n");

    CLEAR_DISP_SCREEN;
    clcd_print("DOWNLOAD DONE", LINE1(0));
    __delay_ms(1000);
    CLEAR_DISP_SCREEN;
}


/* Convert decimal to BCD */
unsigned char to_bcd(unsigned char value)
{
    return ((value / 10) << 4) | (value % 10);
}

/* Convert BCD to decimal */
unsigned char from_bcd(unsigned char value)
{
    return ((value >> 4) * 10) + (value & 0x0F);
}

void set_time(void)
{
    unsigned char hours, minutes, seconds;
    unsigned char key;
    unsigned char field = 0;
    unsigned char blink = 0;
    unsigned char display[9];

    /* Read current RTC time */
    hours = from_bcd(read_ds1307(HOUR_ADDR) & 0x3F);
    minutes = from_bcd(read_ds1307(MIN_ADDR) & 0x7F);
    seconds = from_bcd(read_ds1307(SEC_ADDR) & 0x7F);

    while (1)
    {
        key = read_switches(STATE_CHANGE);

        /* SW1: Increment selected field */
        if (key == MK_SW1)
        {
            if (field == 0)
            {
                hours++;

                if (hours > 23)
                    hours = 0;
            }
            else if (field == 1)
            {
                minutes++;

                if (minutes > 59)
                    minutes = 0;
            }
            else if (field == 2)
            {
                seconds++;

                if (seconds > 59)
                    seconds = 0;
            }
        }

        /* SW2: Change selected field */
        else if (key == MK_SW2)
        {
            field++;

            if (field > 2)
                field = 0;
        }

        /* SW11: Save time and exit */
        else if (key == MK_SW11)
        {
            write_ds1307(SEC_ADDR, to_bcd(seconds));
            write_ds1307(MIN_ADDR, to_bcd(minutes));
            write_ds1307(HOUR_ADDR, to_bcd(hours));

            CLEAR_DISP_SCREEN;
            clcd_print("TIME SAVED", LINE1(0));
            __delay_ms(1000);
            CLEAR_DISP_SCREEN;

            return;
        }

        /* SW12: Cancel without saving */
        else if (key == MK_SW12)
        {
            CLEAR_DISP_SCREEN;
            clcd_print("CANCELLED", LINE1(0));
            __delay_ms(1000);
            CLEAR_DISP_SCREEN;

            return;
        }

        /* Format time */
        display[0] = (hours / 10) + '0';
        display[1] = (hours % 10) + '0';
        display[2] = ':';

        display[3] = (minutes / 10) + '0';
        display[4] = (minutes % 10) + '0';
        display[5] = ':';

        display[6] = (seconds / 10) + '0';
        display[7] = (seconds % 10) + '0';
        display[8] = '\0';

        clcd_print("SET TIME", LINE1(0));

        /* Blink the selected field */
        blink = !blink;

        if (blink)
        {
            if (field == 0)
            {
                display[0] = ' ';
                display[1] = ' ';
            }
            else if (field == 1)
            {
                display[3] = ' ';
                display[4] = ' ';
            }
            else if (field == 2)
            {
                display[6] = ' ';
                display[7] = ' ';
            }
        }

        clcd_print(display, LINE2(4));

        __delay_ms(250);
    }
}


void main(void)
{
    init_config();
   
    while (1)
    {
    key = read_switches(STATE_CHANGE);

    /* Enter menu using SW11 */
    if (key == MK_SW11)
    {
        flagm = 1;
        CLEAR_DISP_SCREEN;
    }
   
    
    if (viewmode == 1)
     {
      if (key == MK_SW1)
      {
         if (index < event_count - 1)
         {
            index++;
          }
     }
     else if (key == MK_SW2)
     {
        if (index > 0)
        {
            index--;
        }
      }
      else if (key == MK_SW12)
      {
        viewmode = 0;
        CLEAR_DISP_SCREEN;
        continue;
      }

    CLEAR_DISP_SCREEN;
    view_log();

    continue;
   }

    else if (flagm == 1)
    {
        main_menu();

        /* Exit menu using SW12 */
        if (key == MK_SW12)
        {
            flagm = 0;
            CLEAR_DISP_SCREEN;
            continue;
        }
        if (key == MK_SW3)
         {
            CLEAR_DISP_SCREEN;
        if (menu == 0)
        {
            index = 0;
            viewmode = 1;
           read_event();
           view_log();
        }
        else if (menu == 1)
        {
            download_log();
        } 
        else if (menu == 2)
        {
            clear_log();
        }
       else if (menu == 3)
       {
        /* Set time */
           set_time();
            CLEAR_DISP_SCREEN;
           
        }
       }

        /* Move down using SW1 */
        if (key == MK_SW1)
        {
            if (menu < 3)
                menu++;

            CLEAR_DISP_SCREEN;
            main_menu();
        }

        /* Move up using SW2 */
        if (key == MK_SW2)
        {
            if (menu > 0)
                menu--;

            CLEAR_DISP_SCREEN;
            main_menu();
        }
    }
    else
    {
        /* Keep your existing dashboard code here */
         
        get_time();
        
        adc_value = read_adc(CHANNEL4);
        speed = ((unsigned long)adc_value * 99UL) / 1023UL;
        
        speed_str[0] =  (unsigned char)(speed / 10) + '0';
        speed_str[1] =  (unsigned char)(speed % 10) + '0';
        speed_str[2] = '\0';

        /* Display labels */
        clcd_print((const unsigned char *)"TIME EVNT SPD", LINE1(0));

        /* Next: format and display time, event and speed */

        clcd_print(time, LINE2(0));
        clcd_print(gear[ind], LINE2(10));
        clcd_print(speed_str, LINE2(14));
  
        if(key == MK_SW1)
        {
            if(ind == 8)
            {
                ind = 1;
            }
            else if(ind < 7)
            {
            ind++;
            }
            store_event();
        }
        if(key == MK_SW2)
        {
            
            if(ind > 0)
            {
            ind--;
            }
            store_event();
            
        }
        if(key == MK_SW3)
        {
            ind = 8;
            store_event();
        }
        
    }
}
    
    return;
}
