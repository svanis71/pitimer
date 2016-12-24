#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

#define CONFIG_COMMAND_READ 0
#define CONFIG_COMMAND_SET 1
#define CONFIG_COMMAND_LIST 2

#define SLEEP_INTERVAL_STRING "sleep_interval"
#define HEARTBEAT_INTERVAL_STRING "heartbeat_interval"
#define CHECK_TEMPRATURE_INTERVAL_STRING "check_temprature_interval"
#define LATITUDE_CONFIG_STRING "latitude"
#define LONGITUDE_CONFIG_STRING "longitude"
#define START_STRING "start"
#define STOP_STRING "stop"
#define WS_URL_STRING "wsStatusUrl"
#define WS_SUNRISESUNSET_URL_STRING "wsSunriseSunsetUrl"

#define LOGGING_STRING "logging"
#define LOGGING_FILE_STRING "file"
#define LOGGING_LEVEL_STRING "level"

#define DEVICE_STRING "device"
#define DEVICE_ID_STRING "id"
#define DEVICE_START_STRING "start"
#define DEVICE_STOP_STRING "stop"
#define DEVICE_USE_SUN_STRING "useSun"

#define COMMENT_TOKEN '#'
#define CONFIG_COMMENT -1
#define SET_DEVICE 0
#define SET_LOGGING 1
#define SET_SLEEP 2
#define SET_HEARTBEAT 3
#define SET_URL 4
#define SET_START 5
#define SET_STOP 6
#define SET_LEVEL 7
#define SET_CHECK_TEMPRATURE_INTERVAL 8
#define SET_SUN_URL 9
#define SET_USE_SUN 10
#define SET_LATITUDE 11
#define SET_LONGITUDE 12

#define UNKNOWN_SETTING 9999

extern int readConfigFile(const char *conf_file);
extern void printConfig(FILE *fp);
extern void initConfig();
extern void config_cleanup();
extern void do_config(const char *conf_file, int command, const char *args);

#endif
