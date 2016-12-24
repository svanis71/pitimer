#ifndef DEVICE_H
#define DEVICE_H

struct Device;

typedef int (*deviceAction)(struct Device*);
typedef void (*updateTimerAction)(struct Device*, const char *start, const char *stop);

typedef struct _TimerConfig 
{
  int startHour;
  int startMinute;
  int stopHour;
  int stopMinute;
  int useSun;
  time_t lastCheck;
  
  void (*dump)(struct _TimerConfig*);
}TimerConfig;

struct Device {
  struct Device *next;
  int id;
  char name[40];
  int status;
  
  TimerConfig config;
  /* public methods (pointers) */
  deviceAction check;
  deviceAction turnOn;
  deviceAction turnOff;
  deviceAction getStatus;
  updateTimerAction updateTimer;
};

extern struct Device *deviceFactory_createDevice(int id, TimerConfig config);

#endif
