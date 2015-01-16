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


#ifndef D_MockIO_H
#define D_MockIO_H


void MockIO_Expect_GPIONR(int led, int bank);
void MockIO_Expect_gpio_output(int port, int state);
void MockIO_ExpectLEDIO(int led, int bank, int val);


#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>
#include <lcd.h>


int IMX_GPIO_NR(int bank, int led);
void gpio_direction_output(int port, int state);
int i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len);
int i2c_write(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len);
void udelay(int msec);
void resetI2CMock();
void epdc_power_on();
void epdc_power_off();
void setup_waveform_file();

void REG_CLR(u32 base, u32 offset, u32 mask);
void REG_SET(u32 base, u32 offset, u32 mask);
void REG_WR(u32 base, u32 offset, u32 value);

void debug(const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif