/*
 * (C) Copyright 2014
 * Printless Plans.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


int probe_pfuze100(void);
int pplans_pmic_write (unsigned char reg, unsigned char value, const char * msg );
int pplans_pmic_read (unsigned char reg, unsigned char  *value, const char * msg );
int pplans_pmic_basic_reg_setup();
int pplans_pmic_sw3_reg_setup();
int pplans_pmic_sw3_independent_op_setup();
int pplans_pmic_sw3_independent_op_check();
int pplans_pmic_handle_sw3 ();