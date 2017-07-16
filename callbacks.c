#include <telldus-core.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

static int running = 1;

void WINAPI deviceEventHandler(int deviceId, int method, const char *data, int callbackId, void *context) {
  printf("Hello %d:%d:%s:%d\n", deviceId, method, data, callbackId);
}

void WINAPI rawDeviceEventHandler(const char *data, int controllerId, int callbackId, void *context) {
  printf("RAW Device: %s\n", data);
}

void WINAPI sensorEventHandler(const char *protocol, const char *model, int id, int dataType, const char *value, int ts, int callbackId, void *context) {
  char timeBuf[80];
  time_t timestamp = ts;

  strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
  /*Print the sensor*/
  printf("SENSOR at %s: %s,\t%s,\t%i\t%s\n", timeBuf, protocol, model, id, value);

  /*Retrieve the values the sensor supports*/
  if (dataType == TELLSTICK_TEMPERATURE) {
    printf("Temperature:\t%sº\t(%s)\n", value, timeBuf);

  } else if (dataType == TELLSTICK_HUMIDITY) {
    printf("Humidity:\t%s%%\t(%s)\n", value, timeBuf);
  }
}


static void sighandler(int sig) {
  switch(sig) {
  case SIGINT:
    printf("Goodbye!\n");
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
