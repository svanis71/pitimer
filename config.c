#include <stdio.h>
#include "mytypes.h"
#include "device_config_list.h"
#include "newconfig.h"

int main(int argc, char **argv)
{
	initConfig();
	readConfigFile();
	printConfig();
	return EXIT_SUCCESS;
}