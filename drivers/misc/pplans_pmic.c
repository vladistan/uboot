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
#include <pplans_pmic.h>


int probe_pfuze100(void)
{
    unsigned char value, dev_id = 0 ;


    if (i2c_read(0x8, 0, 1, &value, 1)) {
        printf("Read device ID error!\n");
        return -1;
    }

    if(value != 0x10 )
    {
        printf("PMIC device id at addr 0x8  [%x]\n", value );
        return -1;
    }

    if (i2c_read(0x8, 3, 1, &value, 1)) {
        printf("Read device revision error!\n");
        return -1;
    }

    if(value != 0x11 )
    {
        printf("PMIC Revison [%x] expected Rev 0x11\n", value );
        return -1;
    }


    printf("PFUZE100 Device: ID: 0x10  REV: 0x11 \n" );

    return 0;
}
