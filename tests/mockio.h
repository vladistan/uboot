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

typedef unsigned char __u8;
typedef         __u8            uint8_t;

void MockIO_Create(int maxExpectations);
void MockIO_Destroy(void);
void MockIO_Expect_GPIONR(int led, int bank);
void MockIO_Expect_gpio_output(int port, int state);
void MockIO_Expect_i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t rv);
void MockIO_Verify_Complete(void);

#ifdef __cplusplus
extern "C" {
#endif


int IMX_GPIO_NR(int bank, int led);
void gpio_direction_output(int port, int state);
int i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len);


#ifdef __cplusplus
}
#endif

#endif