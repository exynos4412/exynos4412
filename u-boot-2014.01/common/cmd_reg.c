#include <common.h>
#include <command.h>
int do_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int addr, val;
	if ((argc < 2) || (argc > 3))
		return cmd_usage(cmdtp);
	addr = simple_strtoul(argv[1], NULL, 16);
	if(2 == argc){
		printf ("##Read Reg[0x%x]=0x%x\n", addr, readl(addr));
		return 0;
	}
	val = simple_strtoul(argv[2], NULL, 16);
	writel(addr, val);
	printf ("##Write Reg[0x%x]=0x%x\n", addr, val);
	return 0;
}

U_BOOT_CMD(
	reg, 3, 1,	do_reg,
	"read/write cpu reg",
	"reg [arg ...]\n    - read/write at address 'arg'\n"
	"       reg arg  -  read arg\n"
	"	reg arg1 arg2  -  write arg2 to arg1\n"
);
