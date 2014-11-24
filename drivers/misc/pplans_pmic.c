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

#define PMIC_CHK if(rv == - 1) {return -1;}


int probe_pfuze100(void)
{
    unsigned char value;
    int rv;


    rv = pplans_pmic_read(0, &value, "device ID"); PMIC_CHK;


    if(value != 0x10 )
    {
        printf("PMIC device id at addr 0x8  [%x]\n", value );
        return -1;
    }

    rv = pplans_pmic_read(3, &value, "device revision"); PMIC_CHK;
    

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

int pplans_pmic_read (unsigned char reg, unsigned char * value, const char * msg )
{

    if (i2c_read(0x8, reg, 1, value, 1)) {
        printf("Read %s error!\n", msg );
        return -1;
    }

    return 0;
}


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


int pplans_pmic_sw3_independent_op_setup() {

    int rv;

    rv = pplans_pmic_write(0x7F, 0x01, "Open extended page 1"); PMIC_CHK;
    rv = pplans_pmic_write(0xB2, 0x0D, "SW3A : Independent, 2MHZ"  ); PMIC_CHK;
    rv = pplans_pmic_write(0xB6, 0x03, "SW3B : Independent, 2MHZ"  ); PMIC_CHK;



    return 0;

}

int pplans_pmic_sw3_independent_op_check() {

    int rv;
    unsigned char value;


    rv = pplans_pmic_write(0x7F, 1, "Open extended page 1"); PMIC_CHK;

    rv = pplans_pmic_read(0xB2, &value, "SW3A : Independent, 2MHZ"  ); PMIC_CHK;
    if ( value != 0x0D ) {
        printf ("SW3A: Independent Not Set. NEED RESTART!!");
        return -1;
    }


    rv = pplans_pmic_read(0xB6, &value, "SW3B : Independent, 2MHZ"  ); PMIC_CHK;
    if ( value != 0x03 ) {
        printf ("SW3B: Independent Not Set. NEED RESTART!!");
        return -1;
    }


    return 0;

}
