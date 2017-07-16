#include <telldus-core.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

static int running = 1;

void WINAPI deviceEventHandler(int deviceId, int method, const char *data, int callbackId, void *context) {
  time_t tiden = time(NULL);
  struct tm *tid = localtime(&tiden);
  fprintf(stdout, "%02d:%02d -- %d:%d:%s:%d\n", tid->tm_hour, tid->tm_min, deviceId, method, data, callbackId);
  fflush(stdout);
}

void WINAPI rawDeviceEventHandler(const char *data, int controllerId, int callbackId, void *context) {
  fprintf(stdout, "RAW Device: %s\n", data);
  fflush(stdout);
}

void WINAPI sensorEventHandler(const char *protocol, const char *model, int id, int dataType, const char *value, int ts, int callbackId, void *context) {
  char timeBuf[80];
  time_t timestamp = ts;

  strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
  /*Print the sensor*/
  fprintf(stdout, "SENSOR at %s: %s,\t%s,\t%i\t%s\n", timeBuf, protocol, model, id, value);
  fflush(stdout);
}


static void sighandler(int sig) {
  switch(sig) {
  case SIGINT:
    fprintf(stdout, "Goodbye!\n");
    fflush(stdout);
    running = 0;
    break;
  }
}
  
int main(int argc, char **argv) {

  static int callbackId[3] = {0, 0, 0};
  
  signal(SIGINT, &sighandler);
  
  tdInit();

  /*Register for callback*/
  callbackId[0] = tdRegisterDeviceEvent( (TDDeviceEvent)&deviceEventHandler, 0 );
  callbackId[1] = tdRegisterRawDeviceEvent( (TDRawDeviceEvent)&rawDeviceEventHandler, 0 );
  callbackId[2] = tdRegisterSensorEvent( (TDSensorEvent)&sensorEventHandler, 0 );

  printf("Waiting...\n");
  fflush(stdout);
  /*Our own simple eventloop*/
  while(running) {
    sleep(1);
  }

  /*Cleanup*/
  tdUnregisterCallback( callbackId[0] );
  tdUnregisterCallback( callbackId[1] );
  tdUnregisterCallback( callbackId[2] );
  tdClose();

  return 0;
  
}
