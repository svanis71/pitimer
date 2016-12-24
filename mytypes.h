#ifndef MYTYPES_H
#define MYTYPES_H

#include <stdio.h>
#include <curl/curl.h>
#include <pthread.h>
#include <telldus-core.h>
#include <time.h>
#include "device.h"

#define ONE 1

#define OFF 0
#define ON 1

#define TRUE_STRING "true"
#define FALSE_STRING "false"

#define TURNON 1
#define TURNOFF 2
#define DO_NOTHING 0

#define MAX_SENSOR_DATALEN 20
#define SENSOR_PROTOCOL "fineoffset"
#define SENSOR_ID_INDOOR 135
#define SENSOR_MODEL_INDOOR "temperaturehumidity"
#define SENSOR_ID_OUTDOOR 136
#define SENSOR_MODEL_OUTDOOR "temperature"

/* Bitmaps */
#define MYTELLSTICK_TEMPRATURE 0x1
#define MYTELLSTICK_HUMIDITY 0x2

#define MYTELLSTICK_SUPPORTED_METHODS TELLSTICK_TURNON | TELLSTICK_TURNOFF

#define CONF_FILE "/etc/jsvtimerd.conf"
#define DEFAULT_LOG_FILE "/usr/local/log/jsvtimerd.log"

#define MAXPATH_LEN 512

#define DEFAULT_WS_URL "http://www.fam-svanstrom.se/api/status"
#define DEFAULT_SUN_WS_URL "http://www.fam-svanstrom.se/api/SunriseSunset?plaintext=true"
#define WS_PARAM_FORMAT "&longitude=%lf&latitude=%lf"

#define TIMER_THREAD_IDX 0
#define WEATHER_THREAD_IDX 1

typedef unsigned char BYTE;

typedef struct LoggInfos {
  FILE *fp;
  char *loggFile;
  int level;
} LoggInfo;

typedef struct _Coordinate {
	double latitude;
	double longitude;
} Coordinate;

typedef struct config {
  int sleepInterval;
  int heartBeatInterval;
  int checkTempratureInterval;
  int useSunsetSunrise;
  LoggInfo logg_config;
  Coordinate homeCoordinate;
} ApplicationConfig;

typedef struct _ResponseData {
  char *data;
  size_t size;
}ResponseData;

typedef struct _requestData {
  const char *content;
  long sizeleft;
}RequestDataStream;

typedef struct _data {
  char trace_ascii; /* 1 or 0 */ 
}TRACE_CONFIG;

typedef struct _WsClient {
  char *url;
  char *sunUrl;
}WsClient;

typedef struct _SensorData {
	int id;
	double temprature;
	double humidity;
}SensorData;

typedef struct ApplicationData {
  ApplicationConfig current_config;
  struct Device *deviceList;
  WsClient ws_client;
  int g__continue;
  int g__reloadFlag;
  int number_of_devices;
  /* 0 = timer loop */
  /* 1 = temprature loop */
  pthread_t worker_thread[2];
  pthread_mutex_t mutex_lock;
/*  SensorData *lastReadSensorData; */
} Application;

/* Global application */  
extern Application g_application;


#endif
