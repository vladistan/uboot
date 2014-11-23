/*
 * Copyright (C) 2012 Printless plans, Inc.
 *
 * Driver for Debug LEDs.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#include <asm/arch-mx6/gpio.h>
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6dl_pins.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/iomux.h>
     
#include <pplans_dbgled.h>
     
     

 void set_debug_led( int led,  int state )
 {

    int bank;
    int pin;

    switch (led)
    {
 	case 1: 	bank = 1;	 pin = 6;	break;
 	case 2: 	bank = 7;	 pin = 12;	break;
 	case 3: 	bank = 1;	 pin = 8;	break;
 	case 4: 	bank = 1;	 pin = 7;	break;
 	case 5: 	bank = 7;	 pin = 13;	break;
 	default: 	bank = 1;	 pin = 6;	break;
    }

    gpio_direction_output(IMX_GPIO_NR(bank, pin), state);


 }