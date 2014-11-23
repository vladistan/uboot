// Name: Gurashish Singh
// Mail: gurashishs@gmail.com
// Date: 6/14/14
// This command allows the user to change the status of the 
// debug LEDs on the PPL board.

#include <common.h>
#include <command.h>


#define NUM_ARGS (2 + 1)




int do_ledset(cmd_tbl_t *cmdtp, int flag, int argc, char * argv[])
{
	uint state, led;
	if (argc < NUM_ARGS) {
		cmd_usage(cmdtp);
		return 1;
	}

	led   = simple_strtoul(argv[1], NULL, 0);
	state = simple_strtoul(argv[2], NULL, 0);

	state = (state)? 1 : 0;

	set_debug_led(led, state);

	return 0;
}

U_BOOT_CMD(
	ledset, NUM_ARGS, 1, do_ledset,
	"ledset allows the user to change the state of one LED on the PPL board.",
	"ledset allows the user to change the state of one LED on the PPL board.\nledset [LED to change {1,2,3,4,5} [0=off 1=on]"
	);
