#include <telldus-core.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define ARG_ON "ON"
#define ARG_OFF "OFF"

#define COMMAND_NOP 0
#define COMMAND_ON 1
#define COMMAND_OFF 2

/* my string lib, just because I can :-) */
void myupper(char *s)
{
	while((*s = toupper(*s)))
		s++;
}

int mystrlen(const char *s)
{
    int len = 0;
    while(*(s + len))
        len++;
    return len;
}

void mystrcpy(char *dest, const char *src)
{
    while( (*dest++ = *src++) );    
}

int mystrcmp(const char *s1, const char *s2)
{
    int ret = 0;
    int offset = 0;
    while(ret == 0 && *(s1 + offset) && *(s2 + offset))
    {
        if(*(s1 + offset) > *(s2 + offset)) ret = 1;
        if(*(s1 + offset) < *(s2 + offset)) ret = -1;
        offset++;
    }
    return ret;
}
/** my string lib **/

int main(int argc, char **argv)
{
	int dev;
	char *command;
	int cmd = COMMAND_NOP;
	
	if(argc == 3) {
		dev = atoi(*(argv + 1));
		int len = mystrlen(*(argv + 2));
        command = (char *)calloc(len + 1, 1);
		mystrcpy(command, *(argv + 2));
		myupper(command);
		printf("Dev: %d\tCommand: %s\n", dev, command);
		if(mystrcmp(ARG_ON, command) == 0) 
			cmd = COMMAND_ON;
		if(mystrcmp(ARG_OFF, command) == 0) 
			cmd = COMMAND_OFF;
		
	}

	tdInit();
	
	printf("Johan listar Telldus devices\n");
	
	int intNumberOfDevices = tdGetNumberOfDevices();
	int supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF;
	int i;

	if(dev > 0)
	  dev--;
	if(cmd == COMMAND_ON) {
	  int devid = tdGetDeviceId(dev);
	  tdTurnOn(devid);
	}
	if(cmd == COMMAND_OFF) {
	  int devid = tdGetDeviceId(dev);
	  tdTurnOff(devid);
	}
	
	printf("Device id\t%-15s\tStatus\n", "Name");
	printf("===================================================================\n");
	for (i = 0; i < intNumberOfDevices; i++) {
	  int id = tdGetDeviceId( i );
	  int methods = tdMethods(id, supportedMethods);
	  int status = tdLastSentCommand(id, methods);
	  char *name = tdGetName( id );
	  
	  printf("%-10d\t%-15s\t", id, name);
	  if(status & TELLSTICK_TURNON) {
	    printf("ON\n");
	  }
	  if(status & TELLSTICK_TURNOFF) {
	    printf("OFF\n");
	  }
	  tdReleaseString(name);
	}
	
	tdClose();
	free(command);
	return 42;
}
