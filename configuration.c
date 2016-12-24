#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "configuration.h"
#include "mytypes.h"
#include "mystring.h"
#include "device.h"
#include "device_config_list.h"

Application g_application;

/** private **/ void saveConfig(const char *conf_file)
{
  FILE *fp_config = fopen(conf_file, "w");
  if(fp_config) {
    fprintf(fp_config, "# Intervals\n");
    fprintf(fp_config, "%s=%d\n", SLEEP_INTERVAL_STRING, g_application.current_config.sleepInterval);
    fprintf(fp_config, "%s=%d\n", HEARTBEAT_INTERVAL_STRING, g_application.current_config.heartBeatInterval);
    fprintf(fp_config, "%s=%d\n", CHECK_TEMPRATURE_INTERVAL_STRING, g_application.current_config.checkTempratureInterval);
    fprintf(fp_config, "# Url to web service\n");
    fprintf(fp_config, "wsUrl=%s\n", g_application.ws_client.url);
    fprintf(fp_config, "wsSunriseSunsetUrl=%s\n", g_application.ws_client.sunUrl);
    fprintf(fp_config, "# Logging\n");
    fprintf(fp_config, "%s.%s=%s\n", LOGGING_STRING, LOGGING_FILE_STRING,
	    g_application.current_config.logg_config.loggFile);
    fprintf(fp_config, "%s.%s=%d\n", LOGGING_STRING, LOGGING_LEVEL_STRING,
	    g_application.current_config.logg_config.level);

    fprintf(fp_config, "# Devices format device.{id-number}.{property}=hh:mm\n");
    for(struct Device *list = g_application.deviceList; list != NULL; list = list->next) {
      fprintf(fp_config, "%s.%d.%s=%02d:%02d\n", DEVICE_STRING, list->id, DEVICE_START_STRING, 
	      list->config.startHour, list->config.startMinute);
      fprintf(fp_config, "%s.%d.%s=%02d:%02d\n", DEVICE_STRING, list->id, DEVICE_STOP_STRING, 
	      list->config.stopHour, list->config.stopMinute);
      fprintf(fp_config, "%s.%d.%s=%s\n", DEVICE_STRING, list->id, DEVICE_USE_SUN_STRING, 
	      list->config.useSun ? "true" : "false");
    }
    fclose(fp_config);
  }
  else
    fprintf(stderr, "Failed to open config file! %s\n", CONF_FILE);
}


/** private **/ int parseKey(const char *key) 
{
  int key_len = mystrlen(key);
  if(key_len > 0 && *key == COMMENT_TOKEN)
    return CONFIG_COMMENT;
  if(strequal(key, SLEEP_INTERVAL_STRING))
    return SET_SLEEP;
  if(strequal(key, HEARTBEAT_INTERVAL_STRING)) 
    return SET_HEARTBEAT;
  if(strequal(key, CHECK_TEMPRATURE_INTERVAL_STRING))
    return SET_CHECK_TEMPRATURE_INTERVAL;
  if(strequal(key, WS_URL_STRING))
    return SET_URL;
  if(strequal(key, WS_SUNRISESUNSET_URL_STRING)) 
    return SET_SUN_URL;
  if(strnequal(key, DEVICE_STRING, mystrlen(DEVICE_STRING))) 
    return SET_DEVICE;
  if(strnequal(key, LOGGING_STRING, mystrlen(LOGGING_STRING)))
    return SET_LOGGING;
  if(strequal(key, LATITUDE_CONFIG_STRING))
    return SET_LATITUDE;
  if(strequal(key, LONGITUDE_CONFIG_STRING))
    return SET_LONGITUDE;
  return UNKNOWN_SETTING;
}

void setUrl(const char *value)
{
  if(g_application.ws_client.url != NULL) {
    g_application.ws_client.url = realloc(g_application.ws_client.url, mystrlen(value) + 1);
    mymemset(g_application.ws_client.url, '\0', sizeof(char));
  }
  else {
    g_application.ws_client.url = (char *)calloc(mystrlen(value) + 1, sizeof(char));
  }
  mystrcpy(g_application.ws_client.url, value);

}

void setSunUrl(const char *value)
{
  if(g_application.ws_client.sunUrl != NULL) {
    g_application.ws_client.sunUrl = realloc(g_application.ws_client.sunUrl, mystrlen(value) + 1);
    mymemset(g_application.ws_client.sunUrl, '\0', sizeof(char));
  }
  else {
    g_application.ws_client.sunUrl = (char *)calloc(mystrlen(value) + 1, sizeof(char));
  }
  mystrcpy(g_application.ws_client.sunUrl, value);
}

void setUseSun(const char *value)
{
  int int_val = 0;
  if(strnequal(value, "true", 4))
    int_val = 1;
  g_application.current_config.useSunsetSunrise = int_val;
}

void setTimerConfigValue(TimerConfig *timer, const char *prop, const char *value)
{
  char *valueCopy = (char *)calloc(mystrlen(value) + 1, sizeof(char));
  mystrcpy(valueCopy, value);
  if(strequal(prop, DEVICE_START_STRING)) {
    char *hour = mystrsep(&valueCopy, ':');
    timer->startHour = atoi(hour);
    timer->startMinute = atoi(valueCopy);
    valueCopy = hour; /* Reset pointer */
  }
  else if(strequal(prop, DEVICE_STOP_STRING)) {
    char *hour = mystrsep(&valueCopy, ':');
    timer->stopHour = atoi(hour);
    timer->stopMinute = atoi(valueCopy);
    valueCopy = hour; /* Reset pointer */
  }
  else if(strequal(prop, DEVICE_USE_SUN_STRING)) {
    timer->useSun = 0;
    if(strnequal("true", value, 4)) 
      timer->useSun = 1;
  }
  free(valueCopy);
}

TimerConfig createTimerConfig() 
{
  TimerConfig itm;
  itm.startHour = 0;
  itm.startMinute = 0;
  itm.stopHour = 0;
  itm.stopMinute = 0;
  itm.useSun = 0;
  itm.lastCheck = 0;
  return itm;
}

void setLogging(const char *key, const char *value)
{
  int propOffset = mystrlen(LOGGING_STRING) + 1;
  char *loggingProperty = (char *)calloc(mystrlen(key) + 1, sizeof(char));
  mystrcpy(loggingProperty, key + propOffset);
  if(strequal(loggingProperty, LOGGING_LEVEL_STRING)) {
    g_application.current_config.logg_config.level = atoi(value);
  }
  else if(strequal(loggingProperty, LOGGING_FILE_STRING)) {
    mystrcpy(g_application.current_config.logg_config.loggFile, value);
  }
  free(loggingProperty);
}

void setDevice(const char *key, const char *value)
{
  int idOffset = mystrlen(DEVICE_STRING) + 1; /* +1 to ignore the dot */
  char *deviceProperty = (char *)calloc(mystrlen(key) + 1, sizeof(char));
  mystrcpy(deviceProperty, key + idOffset);
  char *id_str = mystrsep(&deviceProperty, '.');
  int id = atoi(id_str);
  if(g_application.deviceList == NULL) {
    TimerConfig rootItem = createTimerConfig();
    setTimerConfigValue(&rootItem, deviceProperty, value);
    g_application.deviceList = deviceFactory_createDevice(id, rootItem);
    g_application.number_of_devices = 1;
  }
  struct Device *itm = get_item(g_application.deviceList, id);
  if(itm == NULL) {
    TimerConfig newItem = createTimerConfig();
    setTimerConfigValue(&newItem, deviceProperty, value);
    add_item(g_application.deviceList, deviceFactory_createDevice(id, newItem));
    g_application.number_of_devices++;
  }
  else {
    setTimerConfigValue(&(itm->config), deviceProperty, value);
  }

  free(id_str);
}

/** private **/ void setConfigValue(const char *key, const char *value)
{
  int action = parseKey(key);
  switch(action) {
  case SET_SLEEP:
    g_application.current_config.sleepInterval = atoi(value);
    break;
  case SET_HEARTBEAT:
    g_application.current_config.heartBeatInterval = atoi(value);
    break;
  case SET_CHECK_TEMPRATURE_INTERVAL:
    g_application.current_config.checkTempratureInterval = atoi(value);
    break;
  case SET_URL:
    setUrl(value);
    break;
  case SET_SUN_URL:
    setSunUrl(value);
    break;
  case CONFIG_COMMENT:
    break;
  case SET_DEVICE:
    setDevice(key, value);
    break;
  case SET_LOGGING:
    setLogging(key, value);
    break;
  case SET_USE_SUN:
    setUseSun(value);
    break;
  case SET_LATITUDE:
    g_application.current_config.homeCoordinate.latitude = atof(value);
    break;
  case SET_LONGITUDE:
    g_application.current_config.homeCoordinate.longitude = atof(value);
    break;
  default:
    break;
  }
}

/** private **/ void parseConfig(const char *buf)
{
  char *line = (char *)calloc(128, sizeof(char));
  int offset = 0;

  while(get_line(line, buf + offset, 80)) {
    chomp(line);
    int line_len = mystrlen(line);
    char *key = mystrsep(&line, '=');
    offset = offset + line_len + 1;
    setConfigValue(key, line);
    line = key;
    mymemset(line, 0, 128);
  }
  free(line);
}

/** Config file format 
 ** 
 ** sleepInterval=60
 ** wsUrl=http://localhost/api/status
 ** device.1.start=20:00
 ** device.1.stop=04:20
 ** device.2.start= ...
 **
 **/
int readConfigFile(const char *conf_file) {
  FILE *fp_config = fopen(conf_file, "r");
  if(!fp_config) {
    fprintf(stderr, "Can't open config file %s\n", conf_file);
    return -1;
  }
  
  struct stat fbuf;
  int fd = fileno(fp_config); 
  fstat(fd, &fbuf);
  int size = fbuf.st_size;
  
  char *buf = (char *)calloc(size + 1, sizeof(char));
  int read_bytes = fread(buf, sizeof(char), size, fp_config);
  parseConfig(buf);
  free(buf);
  fclose(fp_config);
  return read_bytes;
}
 

/** public **/ void printConfig(FILE *dest) {
  fprintf(dest, "sleepInterval=%d\n", g_application.current_config.sleepInterval);
  fprintf(dest, "heartBeatInterval=%d\n", g_application.current_config.heartBeatInterval);
  fprintf(dest, "checkTempratureInterval=%d\n", g_application.current_config.checkTempratureInterval);
  fprintf(dest, "loggFile=%s\n", g_application.current_config.logg_config.loggFile);
  fprintf(dest, "logging level=%d\n", g_application.current_config.logg_config.level);
  fprintf(dest, "wsUrl=%s\n", g_application.ws_client.url);
  fprintf(dest, "wsSunUrl=%s\n",  g_application.ws_client.sunUrl);
  struct Device *list =  g_application.deviceList;
  for(; list != NULL; list = list->next) {
    TimerConfig itm = list->config;
    fprintf(dest, "Device id %d\n", list->id);
    fprintf(dest, "\tstart=%02d:%02d\n", itm.startHour, itm.startMinute);
    fprintf(dest, "\tstop=%02d:%02d\n", itm.stopHour, itm.stopMinute);
    fprintf(dest, "\tuseSun=%s\n", itm.useSun ? TRUE_STRING : FALSE_STRING);
  }
}

/** public **/ void initConfig()
{
  /*  g_application.lastReadSensorData = (SensorData *)calloc(1, sizeof(SensorData)); */
  g_application.deviceList = NULL;
  g_application.ws_client.url = NULL;
  g_application.ws_client.sunUrl = NULL;
  g_application.current_config.logg_config.loggFile = (char *)calloc(MAXPATH_LEN, sizeof(char));
  mystrcpy(g_application.current_config.logg_config.loggFile, DEFAULT_LOG_FILE);
  g_application.number_of_devices = 0;
}

/** public **/ void config_cleanup() 
{
  free_list(g_application.deviceList);

  /* Clean up memory */
  if(g_application.ws_client.url)
    free(g_application.ws_client.url);
  if(g_application.ws_client.sunUrl)
    free(g_application.ws_client.sunUrl);
  if(g_application.current_config.logg_config.loggFile)
    free(g_application.current_config.logg_config.loggFile);
}

/** public **/ void do_config(const char *conf_file, int command, const char *args)
{
  readConfigFile(conf_file);
  switch(command) {
  case CONFIG_COMMAND_SET:
    parseConfig(args);
    saveConfig(conf_file);
    break;
  case CONFIG_COMMAND_LIST:
    printConfig(stdout);
    break;
  }
}
