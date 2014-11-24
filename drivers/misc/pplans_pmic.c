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

int pplans_pmic_write (unsigned char reg, unsigned char value, const char * msg )
{

    if (i2c_write(0x8, reg, 1, &value, 1)) {
        printf("%s error!\n", msg );
        return -1;
    }

    return 0;
}

#define PMIC_CHK if(rv == - 1) {return -1;}

int pplans_pmic_basic_reg_setup() {
    int rv;

    rv = pplans_pmic_write(0x6D, 0x1E, "Set VGEN2"); PMIC_CHK;
    rv = pplans_pmic_write(0x6E, 0x10, "Set VGEN3"); PMIC_CHK;
    rv = pplans_pmic_write(0x6F, 0x1D, "Set VGEN4"); PMIC_CHK;
    rv = pplans_pmic_write(0x71, 0x1A, "Set VGEN6"); PMIC_CHK;

    return 0;

}

int pplans_pmic_sw3_reg_setup() {

    int rv;

    rv = pplans_pmic_write(0x3c, 0x20, "Set SW3A Voltage "); PMIC_CHK;
    rv = pplans_pmic_write(0x43, 0x20, "Set SW3B Voltage "); PMIC_CHK;

    return 0;

}