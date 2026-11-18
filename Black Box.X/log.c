/*
 * File:   log.c
 * Author: Radhika
 *
 * Created on 16 October, 2026, 8:56 AM
 */

#include <xc.h>
#include "log.h"
#include "ext eeprom.h"

 unsigned char time[9];
 unsigned char gear[9][3];
unsigned char speed_str[3];

 unsigned int event_count;
 unsigned int ind;
 unsigned char write_index;
 unsigned char event[MAX_EVENTS][16];

void store_event(void)
{
    unsigned int i;
    unsigned int address;

    address = write_index * EVENT_SIZE;

    // Store time: HH:MM:SS
    for (i = 0; i < 8; i++)
    {
        write_ext_eeprom(address++, time[i]);
    }

    // Store gear
    for (i = 0; i < 2; i++)
    {
        write_ext_eeprom(address++, gear[ind][i]);
    }

    // Store speed
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

    if (event_count < MAX_EVENTS)
    {
        event_count++;
    }
}

void read_event(void)
{
    unsigned int i, j;
    unsigned int read_address;
    unsigned int slot;

    for (i = 0; i < event_count; i++)
    {
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