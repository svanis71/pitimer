#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <time.h>
#include <telldus-core.h>
#include <errno.h>

#include "mystring.h"
#include "mytypes.h"
#include "logging.h"
#include "configuration.h"
#include "jsvtellstick.h"
#include "ws_client.h"

void usage(const char *pgmname)
{
  printf("usage: %s [--config-file path to conf] [--config list|set key=value]\n", pgmname);
}

void exit_daemon()
{
  jsvlog_info("Process was terminated.");
  config_cleanup();
  tdClose();
  pthread_mutex_destroy(&g_application.mutex_lock);

  syslog(LOG_NOTICE, "jsvtimerd was terminated");
  closelog();
}

static void sig_handler(int sig)
{
  switch(sig) {
  case SIGHUP:
    g_application.g__reloadFlag = 1;
    jsvlog_info("Recieved SIGHUP");
    break;
  case SIGTERM:
    jsvlog_info("Recieved SIGTERM");
    g_application.g__continue = 0;
    break;
  case SIGABRT:
    syslog(LOG_ERR, "Recieved SIGARBT");
    g_application.g__continue = 0;
    exit_daemon();
    break;
  case SIGSEGV:
    printf("Segmentation fault\n");
    syslog(LOG_ERR, "Recieved SIGSEGV");
    tdClose();
    exit(EXIT_FAILURE);
    g_application.g__continue = 0;
    break;
  }
}

void *timer_loop(void *args)
{
  struct tm tm_now;
  struct tm tm_last_check = now();	
  int action;

  jsvlog_debug("Timer loop starts");
  while (g_application.g__continue) {
    tm_now = now();
    if(tm_now.tm_min != tm_last_check.tm_min) {
      tm_last_check = now();
      for(struct Device *list = g_application.deviceList; list != NULL; list = list->next) {
	if(list->config.useSun) {
	  struct tm *lastWsUpdate = localtime(&list->config.lastCheck);
	  if(tm_now.tm_yday != lastWsUpdate->tm_yday) {
	    jsvlog_debug("Kollar solens tider.");
	    char *ws_result = get_sunset_sunrise(g_application.current_config.homeCoordinate.latitude, 
						 g_application.current_config.homeCoordinate.longitude);
	    char *sunrise = mystrsep(&ws_result, ';');
	    char *sunset = ws_result;
	    jsvlog_debug("Soluppgång: %s, Solnedgång: %s", sunrise, sunset);
	    list->updateTimer(list, sunset, sunrise);
	    ws_result = sunrise;
	    free(ws_result);
	  }
	}
	action = list->check(list);		
	switch(action) {
	case TURNON:
	  jsvlog_info("Device %d was turned ON", list->id);
	  break;
	case TURNOFF:
	  jsvlog_info("Device %d was turned OFF", list->id);
	  break;
	}
      }
    }
    sleep(ONE);
  }
  jsvlog_debug("Out from timer_loop");
  return (void *)1;	
}

void *weather_loop(void *args)
{
  int arr_size = g_application.number_of_devices;
  int *device_array_status = (int *)calloc(arr_size, sizeof(int));
  int checkTemprature = 0;
  jsvlog_debug("weather_loop starts");

  while (g_application.g__continue) {
    checkTemprature = (checkTemprature + 1) % g_application.current_config.checkTempratureInterval;
    if(checkTemprature == 0) {
      jsvlog_debug("check the tempratures");
      double it = getIndoorTemprature();
      double ot = getOutdoorTemprature();
      double ih = getIndoorHumidity();

      if(g_application.number_of_devices > arr_size) {
	syslog(LOG_NOTICE, "new devices has been added.");
	device_array_status = (int *)realloc(device_array_status, (g_application.number_of_devices - arr_size) * sizeof(int));
      }
			
      jsvlog_debug("check current status");
      getDevicesStatus(device_array_status);
      jsvlog_debug("post current tempratures");
      /*post_current_status(it, ot, ih, device_array_status, g_application.number_of_devices);*/
    }
		
    sleep(ONE);
  }	
  jsvlog_debug("Out from weather_loop");
  free(device_array_status);
  return (void *)1;
}

void dumpConfig() 
{
  FILE *dumpFile = fopen("/usr/local/logs/config.dmp", "w");
  printConfig(dumpFile);
  fclose(dumpFile);
}

void daemon_loop(const char *conf_file)
{
  int isAliveMonitor = 0;
  pid_t mypid = getpid();

  char *procFile = (char *) calloc(100, sizeof(char));
  sprintf(procFile, "/proc/%d/status", mypid);
  
  jsvlog_debug("Enter main loop");
  jsvlog_debug("heartbeat interval is %d", g_application.current_config.heartBeatInterval);
  while (g_application.g__continue) {
	  
    if(g_application.g__reloadFlag) {
      g_application.g__reloadFlag = 0;
      syslog(LOG_NOTICE, "daemon got a reload signal.");
      readConfigFile(conf_file);
      dumpConfig();
      syslog(LOG_NOTICE, "configuration was reloaded.");
    }
		
    if(isAliveMonitor == 0) {
      syslog(LOG_NOTICE, "jsvtimerd is alive.");
      FILE *status = fopen(procFile, "r");
      char line[80];
      while(fgets (line, 80, status) != NULL) {
	if(strnequal(line, "VmSize", 6)) {
	  chomp(line);
	  jsvlog_info("%s", line);
	  break;
	}
      }
      fclose(status);
    }
    sleep(ONE);
    isAliveMonitor = (isAliveMonitor + 1) % g_application.current_config.heartBeatInterval;
  }
  jsvlog_debug("Out of the main loop!");
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

int main(int argc, char **argv) {
  pid_t pid, sid;
  int configCommand = CONFIG_COMMAND_READ;
  int hasConfFile = 0;
  char *configArg = (char *)calloc(200, 1);
  char *conf_file = (char *)calloc(256, 1);

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
  printf("Config file is: %s\n", conf_file);
  
  /* Clone ourselves to make a child */
  pid = fork(); 

  /* If the pid is less than zero,
   *    something went wrong when forking */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* If the pid we got back was greater
   *    than zero, then the clone was
   *       successful and we are the parent. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* TZ='Europe/Stockholm'; export TZ */
  char timezone[3];
  char region[20];
  mystrcpy(timezone, "TZ");
  mystrcpy(region, "Europe/Stockholm");
  setenv(timezone, region, 1);

  /** Init the tellstick **/
  tdInit();

  /* New config stuff */
  initConfig();
   
  do_config(conf_file, configCommand, configArg);
  free(conf_file);
  free(configArg);

  if(configCommand == CONFIG_COMMAND_LIST || configCommand == CONFIG_COMMAND_SET)
    exit(EXIT_SUCCESS);
  /********************/
  
  g_application.g__continue = 1;
  g_application.g__reloadFlag = 0;

  /* Init the mutex flag */
  if(pthread_mutex_init(&g_application.mutex_lock, NULL) != 0) {
    char err[100];
    sprintf(err, "Failed to create mutex. error: (0x%X)", errno);
    syslog(LOG_ERR, err);
    exit(EXIT_FAILURE);
  }
	

  int log_ok =  init_logging(g_application.current_config.logg_config.loggFile); /* Must be done after fork and after config was read */
  if(!log_ok) {
    fprintf(stderr, "Failed to init log engine. Program will stop.\n");
    exit(EXIT_FAILURE);
  }
  printf("Process started with pid %d\n", getpid());
  
  /* If execution reaches this point we are the child */
  /* Set the umask to zero */
  umask(0);

  /* Open a connection to the syslog server */
  openlog(argv[0],LOG_NOWAIT|LOG_PID,LOG_USER); 

  /* Sends a message to the syslog daemon */
  syslog(LOG_NOTICE, "Successfully started daemon"); 

  /* Try to create our own process group */
  sid = setsid();
  if (sid < 0) {
    syslog(LOG_ERR, "Could not create process group");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    printf("Failed to cd /\n");
    syslog(LOG_ERR, "Could not change working directory to /");
    exit(EXIT_FAILURE);
  }

  signal(SIGTERM, sig_handler);
  signal(SIGKILL, sig_handler);
  signal(SIGHUP, sig_handler);
  signal(SIGABRT, sig_handler);
  signal(SIGSEGV, sig_handler);
	
  /* Define worker threads */
  
  int create_thread_status = 0;
  create_thread_status = pthread_create(&g_application.worker_thread[TIMER_THREAD_IDX], NULL, timer_loop, NULL);
  if(create_thread_status) {
    char err[100];
    sprintf(err, "Failed to create timer thread. error: (0x%X)", errno);
    syslog(LOG_ERR, err);
    exit(EXIT_FAILURE);		
  }
  pthread_create(&g_application.worker_thread[WEATHER_THREAD_IDX], NULL, weather_loop, NULL);
  if(create_thread_status) {
    char err[100];
    sprintf(err, "Failed to create weather thread. error: (0x%X)", errno);
    syslog(LOG_ERR, err);
    exit(EXIT_FAILURE);		
  }
  	
  /* Main program loop */
  jsvlog_info("Process started with pid %d", getpid());
  /*  g_application.pid = getpid();*/

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  syslog(LOG_NOTICE, conf_file);
  daemon_loop(conf_file);
  exit_daemon();
  close_log();
}

