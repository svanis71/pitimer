#include <stdlib.h>
#include <time.h>
#include <telldus-core.h>
#include "mytypes.h"
#include "logging.h"
#include "mystring.h"

struct Clock {
  int hour;
  int minute;
};

struct tm device__now()
{
  time_t time_sec;
  struct tm *info;
  time(&time_sec);
  info = localtime(&time_sec);

  return *info;
}

struct Clock getStartStopTime(const char *stringTime) {
  struct Clock clock;
  char *valueCopy = (char *)calloc(mystrlen(stringTime) + 1, sizeof(char));
  mystrcpy(valueCopy, stringTime);
  char *strHour = mystrsep(&valueCopy, ':');
  clock.hour = atoi(strHour);
  clock.minute = atoi(valueCopy);
  valueCopy = strHour; /* Reset pointer */
  free(valueCopy);
  return clock;
}

void device__updateTimer(struct Device *device, const char *startTime, const char *stopTime) {
  jsvlog_debug("Update timer in. Id: %d New start: %s New stop: %s", device->id, startTime, stopTime);
  struct tm now = device__now();
  struct Clock start = getStartStopTime(startTime);
  struct Clock stop = getStartStopTime(stopTime);
	
  device->config.startHour = start.hour;
  device->config.startMinute = start.minute;
  device->config.stopHour = stop.hour;
  device->config.stopMinute = stop.minute;
  device->config.lastCheck = mktime(&now);
	
}

int device__turnOn(struct Device *device) {
  jsvlog_debug("Turn on device %d", device->id);
  int supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF;
  int id = tdGetDeviceId(device->id - 1);
  int methods = tdMethods(id, supportedMethods);
  if(methods & TELLSTICK_TURNON) {
    tdTurnOn(id);
  }
  return 1;
}

int device__turnOff(struct Device *device) {
  jsvlog_debug("Turn off device %d", device->id);
  int supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF;
  int id = tdGetDeviceId(device->id - 1);
  int methods = tdMethods(id, supportedMethods);
  if(methods & TELLSTICK_TURNOFF) {
    tdTurnOff(id);
  }
  return 1;
}

int device__check(struct Device *device) {
  if(!device) {
    jsvlog_error("Holy smoke, device was NULL :'-(");
    return DO_NOTHING;
  }
  struct tm info = device__now();
  jsvlog_debug("check device %d at %04d%02d%02d %02d:%02d", device->id, info.tm_year + 1900, info.tm_mon + 1, info.tm_mday,
	       info.tm_hour, info.tm_min);
  if(info.tm_hour == device->config.startHour && info.tm_min == device->config.startMinute) {
    device->turnOn(device);
    return TURNON;
  }
  if(info.tm_hour == device->config.stopHour && info.tm_min == device->config.stopMinute) {
    device->turnOff(device);
    return TURNOFF;
  }
  return DO_NOTHING;
}

int device__getStatus(struct Device *device)
{
  int id = tdGetDeviceId(device->id - 1);
  int methods = tdMethods(id, MYTELLSTICK_SUPPORTED_METHODS);
  int status = tdLastSentCommand(id, methods);
  device->status = status & TELLSTICK_TURNON ? ON : OFF;
  return device->status;
}

struct Device *deviceFactory_createDevice(int id, TimerConfig config) {
  struct Device *device = (struct Device *)malloc(sizeof(struct Device));
  device->id = id;
  device->config.startHour = config.startHour;
  device->config.startMinute = config.startMinute;
  device->config.stopHour = config.stopHour;
  device->config.stopMinute = config.stopMinute;
	
  device->turnOn = &device__turnOn;
  device->turnOff = &device__turnOff;
  device->check = &device__check;
  device->getStatus = &device__getStatus;
  device->updateTimer = &device__updateTimer;
  return device;
}
