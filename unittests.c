#include <stdio.h>
#include <stdlib.h>
#include "mytypes.h"
#include "mystring.h"
#include "device_config_list.h"
#include "configuration.h"
#include "ws_client.h"

#define TESTSTRING "Hello, world!"

void usage(const char *pgmname)
{
  printf("usage: %s [config] [<configkey> <configvalue>]\n", pgmname);
}

void test_strcpy() {
  char *actual = (char *)calloc(20, 1);
  mystrcpy(actual, TESTSTRING);
  printf("mystrcpy: expected: '%s' was '%s'\n", TESTSTRING, actual);
  free(actual);
}


void parseArgvConfig(int argc, char **argv, int offset, int *configCommand, char *configArgs)
{
  if(strequal(*(argv + offset), "--config") || strequal(*(argv + offset), "-c")) {
    if(argc < offset + 1) {
      usage(*argv);
      exit(EXIT_FAILURE);
    }
    else {
      offset++;
      if(strequal(*(argv + offset), "list")) {
	*configCommand = CONFIG_COMMAND_LIST;
      } 
      else {
	if(strequal(*(argv + offset), "set")) {
	  *configCommand = CONFIG_COMMAND_SET;
	  if(argc < offset + 1) {
	    usage(*argv);
	    exit(EXIT_FAILURE);
	  }
	  else {
	    offset++;
	    mystrcpy(configArgs, *(argv + offset));
	  }
	}
      }
    }
  }
}

int main(int argc, char **argv)
{
  int configCommand = CONFIG_COMMAND_READ;
  int hasConfFile = 0;
  char *configArg = (char *)calloc(200, 1);
  char *conf_file = (char *)calloc(256, 1);

  tdInit();
  if(argc > 1) {
    for(int i = 1; i < argc; i++) {
      parseArgvConfig(argc, argv, i, &configCommand, configArg);
      if(strequal(*(argv + i), "--config-file") || strequal(*(argv + i), "-f")) {
	hasConfFile = 1;
	if(argc < i + 1) {
	  printf("Fail!\n");
	}
	else {
	  i++;
	  mystrcpy(conf_file, *(argv + i));
	}
      }
    }
  }

  if(hasConfFile != 1) 
    mystrcpy(conf_file, CONF_FILE);
  /* New config stuff */
  initConfig();
  do_config(conf_file, configCommand, configArg);
   printf("Hej igen: %s\n", conf_file);

  test_strcpy();
	
  struct Device *d = get_item(g_application.deviceList, 1);
  printf("start %02d:%02d\tuseSun:%d\n", d->config.startHour, d->config.startMinute, d->config.useSun);

  printf("Ställer in mot solen...\n");
  printf("Coords: Lat: %lf\tLng: %lf\n", g_application.current_config.homeCoordinate.latitude, g_application.current_config.homeCoordinate.longitude);

  char *ws_result = get_sunset_sunrise(g_application.current_config.homeCoordinate.latitude, 
				       g_application.current_config.homeCoordinate.longitude);
  printf("Från WS: %s\n", ws_result);
  char *sunrise = mystrsep(&ws_result, ';');
  char *sunset = ws_result;
  printf("Sunrise: %s\n", sunrise);
  printf("Sunset: %s\n", sunset);
  if(d->updateTimer != NULL)  {
    printf("UpdateTimer %ld\n", d->updateTimer);
    d->updateTimer(d, sunset, sunrise);
  }
  else
    printf("updateTimer is NULL we'll crasch!\n");
   
  ws_result = sunrise;
  free(ws_result);

  printf("getStatus before = %d\n", d->getStatus(d));
  d->turnOff(d);
  printf("getStatus after off = %d\n", d->getStatus(d));
  d->turnOn(d);
  printf("getStatus after on= %d\n", d->getStatus(d));

  printf("# of devices %d\n", g_application.number_of_devices);
  int *devs = (int *)calloc(g_application.number_of_devices, sizeof(int));
  getDevicesStatus(devs);
  printf("Status flags: #");
  for(int i = 0; i < g_application.number_of_devices; i++) {
    printf("%d", devs[i]);
  }
  puts("#");
  printf("Anropar WS...");
  fflush(stdout);
  char *ws_result2 = get_sunset_sunrise(56.21227, 15.542849);
  printf("\nSvaret blev: %s\n", ws_result2);
  free(ws_result2);
  tdClose();
  config_cleanup();
  for(int i = 0; i < argc; i++) {
    printf("*(argv + %d) = %s\n", i, *(argv + i));
  }

}
