#include <stdlib.h>
#include <time.h>
#include <telldus-core.h>
#include "mytypes.h"
#include "mystring.h"
#include "logging.h"
#include "ws_client.h"

/*
FILE *fp;
int g__continue;
int g__reloadFlag;
struct config current_config;
*/

struct tm now()
{
	time_t time_sec;
	struct tm *info;
	time(&time_sec);
	info = localtime(&time_sec);

	return *info;
}

void turnOnAll()
{
	int supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF;
	int intNumberOfDevices = tdGetNumberOfDevices();
	for (int i = 0; i < intNumberOfDevices; i++) {
		int id = tdGetDeviceId( i );
		int methods = tdMethods(id, supportedMethods);
		if(methods & TELLSTICK_TURNON) {
			tdTurnOn(id);
		}
	}	
}

void turnOffAll()
{
	int supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF;
	int intNumberOfDevices = tdGetNumberOfDevices();
	for (int i = 0; i < intNumberOfDevices; i++) {
		int id = tdGetDeviceId( i );
		int methods = tdMethods(id, supportedMethods);
		if(methods & TELLSTICK_TURNOFF) {
			tdTurnOff(id);
		}
	}
}

double getIndoorTemprature()
{
  int ret;
  int ts;
  char *val = (char *)calloc(MAX_SENSOR_DATALEN, sizeof(char));
  float temp = -273.0f;

  ret = tdSensorValue(SENSOR_PROTOCOL, SENSOR_MODEL_INDOOR, SENSOR_ID_INDOOR, MYTELLSTICK_TEMPRATURE, val, MAX_SENSOR_DATALEN, &ts);
  if(ret == TELLSTICK_SUCCESS) {
    temp = atof(val);   
  }
	free(val);
  return temp;
}

double getIndoorHumidity()
{
  int ret;
  int ts;
  char *val = (char *)calloc(MAX_SENSOR_DATALEN, sizeof(char));
  float humid = 100.0f;

  ret = tdSensorValue(SENSOR_PROTOCOL, SENSOR_MODEL_INDOOR, SENSOR_ID_INDOOR, MYTELLSTICK_HUMIDITY, val, MAX_SENSOR_DATALEN, &ts);
  if(ret == TELLSTICK_SUCCESS) {
    humid = atof(val);
  }
	free(val);
  return humid;   
}

double getOutdoorTemprature()
{
  int ret;
  int ts;
  char *val = (char *)calloc(MAX_SENSOR_DATALEN, sizeof(char));
  float temp = -273.0f;

  ret = tdSensorValue(SENSOR_PROTOCOL, SENSOR_MODEL_OUTDOOR, SENSOR_ID_OUTDOOR, MYTELLSTICK_TEMPRATURE, val, MAX_SENSOR_DATALEN, &ts);
  if(ret == TELLSTICK_SUCCESS) {
    temp = atof(val);
  }
	free(val);
  return temp;
}

void getDevicesStatus(int *ptr)
{
  struct Device *list = g_application.deviceList;
  for(int i = 0; list != NULL ; list = list->next, i++) {
	  int id = tdGetDeviceId(list->id);
		int methods = tdMethods(id, MYTELLSTICK_SUPPORTED_METHODS);
		int status = tdLastSentCommand(id, methods);
		ptr[i] = status & TELLSTICK_TURNON ? ON : OFF;
	}
}


