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
  printf("usage: %s [config] [<configkey> <configvalue>]\n", pgmname);
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
  case SIGKILL:
    jsvlog_info("Recieved SIGKILL");
    g_application.g__continue = 0;
    break;
  }
}

void *timer_loop(void *args)
{
  struct tm tm_now;
  struct tm tm_last_check = now();	

  while (g_application.g__continue) {
    tm_now = now();
    if(tm_now.tm_min != tm_last_check.tm_min) {
      tm_last_check = now();
      int action = check(g_application.current_config.timer_config->startHour, 
			 g_application.current_config.timer_config->startMinute, 
			 g_application.current_config.timer_config->stopHour, 
			 g_application.current_config.timer_config->stopMinute);
      do_action(action);
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

  while (g_application.g__continue) {
    checkTemprature = (checkTemprature + 1) % 30; /* TODO: g_application.current_config.checkTemratureInterval */
    if(checkTemprature == 0) {
      /* TODO: Read and send temprature */
      double it = getIndoorTemprature();
      double ot = getOutdoorTemprature();
      double ih = getIndoorHumidity();

      if(g_application.number_of_devices > arr_size) {
	syslog(LOG_NOTICE, "new devices has been added.");
	device_array_status = (int *)realloc(device_array_status, (g_application.number_of_devices - arr_size) * sizeof(int));
      }
			
      getDevicesStatus(device_array_status, g_application.number_of_devices);
      post_current_status(it, ot, ih, device_array_status, g_application.number_of_devices);
    }
		
    sleep(ONE);
  }	
  jsvlog_debug("Out from weather_loop");
  free(device_array_status);
  return (void *)1;
}

void daemon_loop()
{
  int isAliveMonitor = 0;
  int dbg_loops = 1200;

  while (g_application.g__continue) {
	  
    if(g_application.g__reloadFlag) {
      g_application.g__reloadFlag = 0;
      syslog(LOG_NOTICE, "daemon got a reload signal.");
      readConfigFile();
      syslog(LOG_NOTICE, "configuration was reloaded.");
    }
		
    if(isAliveMonitor == 0) {
      syslog(LOG_NOTICE, "jsvtimerd is alive.");
    }
    sleep(ONE);
    isAliveMonitor = (isAliveMonitor + 1) % g_application.current_config.heartBeatInterval;
    dbg_loops = dbg_loops - 1;
    if(dbg_loops == 0)
      g_application.g__continue = 0;
  }
  jsvlog_info("Out of the loop!");
}

int main(int argc, char **argv) {
  pid_t pid, sid;

  initConfig();
  /** Init the tellstick **/
  tdInit();
  
  if(argc > 1) {
    int cmp = mystrcmp(*(argv + 1), "config");
    if(!cmp) {
      int res = do_config(argc, argv);
      exit(res < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    else{
      usage(*argv);
      exit(EXIT_FAILURE);
    }
  }
	
  readConfigFile();
  g_application.g__continue = 1;
  g_application.g__reloadFlag = 1;
  
  /*  printf("Temp ute: %.2lf\tTemp inne: %.2lf\tLuftfuktighet: %.2lf\n", ot, it, ih);*/

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

  printf("Process started with pid %d\n", getpid());

  /* If execution reaches this point we are the child */
  /* Set the umask to zero */
  umask(0);

  /* Open a connection to the syslog server */
  openlog(argv[0],LOG_NOWAIT|LOG_PID,LOG_USER); 

  /* Sends a message to the syslog daemon */
  syslog(LOG_NOTICE, "Successfully started daemon\n"); 

  /* Try to create our own process group */
  sid = setsid();
  if (sid < 0) {
    syslog(LOG_ERR, "Could not create process group\n");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    printf("Failed to cd /\n");
    syslog(LOG_ERR, "Could not change working directory to /\n");
    exit(EXIT_FAILURE);
  }

  /* Close the standard file descriptors 
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);*/

  if(!(g_application.current_config.logg_config.fp = fopen(g_application.current_config.logg_config.loggFile, "a"))) {
    syslog(LOG_ERR, "Failed to create log file\n");
    exit(EXIT_FAILURE);
  }

  jsvlog_info("Process started with pid %d", getpid());
  signal(SIGTERM, sig_handler);
  signal(SIGKILL, sig_handler);
  signal(SIGHUP, sig_handler);
	
  /* TZ='Europe/Stockholm'; export TZ */
  char timezone[3];
  char region[20];
  mystrcpy(timezone, "TZ");
  mystrcpy(region, "Europe/Stockholm");
  setenv(timezone, region, 1);

  /* Init the mutex flag */
  if(pthread_mutex_init(&g_application.mutex_lock, NULL) != 0) {
    char err[100];
    sprintf(err, "Failed to create mutex. error: (0x%X)", errno);
    syslog(LOG_ERR, err);
    exit(EXIT_FAILURE);
  }
	
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
  daemon_loop();

  jsvlog_info("Process was terminated.");
  config_cleanup();
  tdClose();
  pthread_mutex_destroy(&g_application.mutex_lock);
	
  syslog(LOG_NOTICE, "jsvtimerd was terminated");
  closelog();
	
}

