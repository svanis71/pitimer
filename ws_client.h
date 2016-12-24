#ifndef WS_CLIENT_H
#define WS_CLIENT_H

extern void init_curl();
extern void curl_cleanup();
extern void post_current_status(double indoor_temprature, double outdoor_temprature, double indoor_humidity, int *statusArray, int number_of_devices);
extern void get_changes_from_ws(int *device_array_status);
extern char *get_sunset_sunrise(double lat, double lng);

#endif
