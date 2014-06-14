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

int do_dgprint(cmd_tbl_t *cmdtp, int flag, int argc, char * argv[])
{
	printf("Hi This is a test\n");

	gpio_direction_output(IMX_GPIO_NR(1,8),1);
	gpio_direction_output(IMX_GPIO_NR(1,7),1);
	gpio_direction_output(IMX_GPIO_NR(1,6),1);
	gpio_direction_output(IMX_GPIO_NR(1,9),1);
	// mxc_set_gpio_direction(MX6DL_PAD_GPIO_8__GPIO_1_8, 0);
	// mxc_set_gpio_dataout(MX6DL_PAD_GPIO_8__GPIO_1_8, 1);
	printf("Hi This is a test\n");
	return 0;
}

U_BOOT_CMD(
	dgprint, 1, 1, do_dgprint,
	"dgprint takes no arguements",
	"this is just to test a function"
	);