// Name: Gurashish Singh
// Mail: gurashishs@gmail.com
// Date: 6/14/14
// This command allows the user to change the status of the 
// debug LEDs on the PPL board.

#include <common.h>
#include <command.h>

#include <asm/arch-mx6/gpio.h>
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6dl_pins.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/iomux.h>

#define NUM_ARGS (2 + 1)


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


int do_ledset(cmd_tbl_t *cmdtp, int flag, int argc, char * argv[])
{
	uint state, led;
	if (argc < NUM_ARGS) {
		cmd_usage(cmdtp);
		return 1;
	}

	led   = simple_strtoul(argv[1], NULL, 10);
	state = simple_strtoul(argv[2], NULL, 10);

	state = (state)? 1 : 0;

	set_debug_led(led, state);

	return 0;
}

U_BOOT_CMD(
	ledset, NUM_ARGS, 1, do_ledset,
	"ledset allows the user to change the state of one LED on the PPL board.",
	"ledset allows the user to change the state of one LED on the PPL board.\nledset [LED to change {1,2,3,4,5} [0=off 1=on]"
	);
