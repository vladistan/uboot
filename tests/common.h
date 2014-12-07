/**
  Test doubles for LED operations


**/

typedef void cmd_tbl_t ;
typedef unsigned int uint;
typedef unsigned char __u8;
typedef         __u8            uint8_t;

/* Uboot ENV */
void cmd_usage(void *);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
#define U_BOOT_CMD(name, args, n, exec, short_help, long_help )


/* Hardware */
int IMX_GPIO_NR(int bank, int led);
void gpio_direction_output(int port, int state);

/* Verifies */



/* Components under test */
void set_debug_led( int led,  int state );
void set_debug_led_bank(int pattern);
int do_ledset(cmd_tbl_t *cmdtp, int flag, int argc, char * argv[]);
int i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len);
int i2c_write(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len);
void udelay(int msec);


#include <stdlib.h>
#include <stdio.h>
