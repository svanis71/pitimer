#ifndef JSVTELLSTICK_H
#define JSVTELLSTICK_H

extern struct tm now();
extern double getIndoorTemprature();
extern double getIndoorHumidity();
extern double getOutdoorTemprature();
extern int getDevicesStatus(int *ptr);
#endif
