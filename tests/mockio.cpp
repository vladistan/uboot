/***
 * Excerpted from "Test-Driven Development for Embedded C",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material, 
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose. 
 * Visit http://www.pragmaticprogrammer.com/titles/jgade for more book information.
***/
/*- ------------------------------------------------------------------ -*/
/*-    Copyright (c) James W. Grenning -- All Rights Reserved          -*/
/*-    For use by owners of Test-Driven Development for Embedded C,    -*/
/*-    and attendees of Renaissance Software Consulting, Co. training  -*/
/*-    classes.                                                        -*/
/*-                                                                    -*/
/*-    Available at http://pragprog.com/titles/jgade/                  -*/
/*-        ISBN 1-934356-62-X, ISBN13 978-1-934356-62-3                -*/
/*-                                                                    -*/
/*-    Authorized users may use this source code in your own           -*/
/*-    projects, however the source code may not be used to            -*/
/*-    create training material, courses, books, articles, and         -*/
/*-    the like. We make no guarantees that this source code is        -*/
/*-    fit for any purpose.                                            -*/
/*-                                                                    -*/
/*-    www.renaissancesoftware.net james@renaissancesoftware.net       -*/
/*- ------------------------------------------------------------------ -*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "mockio.h"
#include "CppUTest/TestHarness_c.h"
#include <CppUTestExt/MockSupport_c.h>
    

int IMX_GPIO_NR(int bank, int led)
{
    
      int rv = (bank << 8) | led;
      mock_c()->actualCall("IMX_GPIO_NR")->withIntParameters( "bank", bank)->withIntParameters("led", led)->
          returnValue();
    
      return rv;
}

void setup_waveform_file()
{
    mock_c()->actualCall("epdc_power_on");
}

void epdc_power_on()
{
    mock_c()->actualCall("epdc_power_on");
}

void epdc_power_off()
{
    mock_c()->actualCall("epdc_power_off");
}

void REG_CLR(u32 base, u32 offset, u32 mask)
{
    mock_c()->actualCall("REG_CLR")
            ->withIntParameters("base", base)
            ->withIntParameters("offset", offset)
            ->withIntParameters("mask", mask);
}

void REG_SET(u32 base, u32 offset, u32 mask)
{
    mock_c()->actualCall("REG_SET")
            ->withIntParameters("base", base)
            ->withIntParameters("offset", offset)
            ->withIntParameters("mask", mask);
}

void REG_WR(u32 base, u32 offset, u32 value)
{
    mock_c()->actualCall("REG_WR")
            ->withIntParameters("base", (unsigned int) base)
            ->withIntParameters("offset", (unsigned int) offset)
            ->withIntParameters("value",  (unsigned int) value);
}

__u32 REG_RD(u32 base, u32 offset)
{

    //regrd_read_rq_count++;

    int rv = mock_c()->actualCall("REG_RD")
            ->withIntParameters("base", base)
            ->withIntParameters("offset", offset)
            ->returnValue().value.intValue;


    return rv;
}

vidinfo_t panel_info;

void debug(const char * format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}



void gpio_direction_output(int port, int state)
{

    mock_c()
        ->actualCall("gpio_direction_output")
        ->withIntParameters( "port", port)
        ->withIntParameters("state", state)
        ->returnValue();
  

}


int i2c_read_rq_count = 0;

void resetI2CMock(){
    i2c_read_rq_count = 0;
}

int i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len)
{
     
    i2c_read_rq_count++;
    
    int rv = mock_c()
        ->actualCall("i2c_read")
        ->withIntParameters("chip", 0x8)
        ->withIntParameters("addr", addr)
        ->returnValue().value.intValue;
       
    char X[80];
    sprintf(X,"I2C:RD:%d:%d:%d", chip, addr, i2c_read_rq_count  );
    
    int val = mock_c()->getData(X).value.intValue;
    
    buffer[0] = (unsigned)val;
    
    return rv;
}


int i2c_write(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len)
{
    
    int rv = mock_c()
        ->actualCall("i2c_write")
        ->withIntParameters("chip", 0x8)
        ->withIntParameters("addr", addr)
        ->withIntParameters("val", buffer[0])
        ->returnValue().value.intValue;

    return rv;
}


void udelay(int msec)
{

}




