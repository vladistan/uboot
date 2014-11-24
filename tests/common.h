/**
  Test doubles for LED operations


**/

typedef void cmd_tbl_t ;
typedef unsigned int uint;

/* Uboot ENV */
void cmd_usage(void *);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
#define U_BOOT_CMD(name, args, n, exec, short_help, long_help )


/* Hardware */
int IMX_GPIO_NR(int bank, int led);
void gpio_direction_output(int port, int state);

/* Verifies */
void reset_verify();
int verify_IMX_GPIO_NR(int bank, int led);
int verify_gpio_direction_output(int port, int state);
int verify_cmd_usage(void *);


/* Components under test */
void set_debug_led( int led,  int state );
void set_debug_led_bank(int pattern);
int do_ledset(cmd_tbl_t *cmdtp, int flag, int argc, char * argv[]);


#include <stdlib.h>
#include <stdio.h>
