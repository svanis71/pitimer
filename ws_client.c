#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "mytypes.h"
#include "mystring.h"
#include "jsvtellstick.h"
#include "logging.h"
 
static void dump(const char *text,
		 FILE *stream, unsigned char *ptr, size_t size,
		 char nohex)
{
  size_t i;
  size_t c;
 
  unsigned int width=0x10;
 
  if(nohex)
    /* without the hex output, we can fit more on screen */ 
    width = 0x40;
 
  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
          text, (long)size, (long)size);
 
  for(i=0; i<size; i+= width) {
 
    fprintf(stream, "%4.4lx: ", (long)i);
 
    if(!nohex) {
      /* hex not disabled, show it */ 
      for(c = 0; c < width; c++)
        if(i+c < size)
          fprintf(stream, "%02x ", ptr[i+c]);
        else
          fputs("   ", stream);
    }
 
    for(c = 0; (c < width) && (i+c < size); c++) {
      /* check for 0D0A; if found, skip past and start a new line of output */ 
      if (nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
        i+=(c+2-width);
        break;
      }
      fprintf(stream, "%c",
              (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */ 
      if (nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
        i+=(c+3-width);
        break;
      }
    }
    fputc('\n', stream); /* newline */ 
  }
  fflush(stream);
}
 
static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  FILE *trace_fp = fopen("/usr/local/log/ws_client.trc", "a");
  TRACE_CONFIG *config = (TRACE_CONFIG *)userp;
  const char *text;
  (void)handle; /* prevent compiler warning */ 
 
  switch (type) {
  case CURLINFO_TEXT:
    jsvlog_error("== Info: %s", data);
  default: /* in case a new one is introduced to shock us */ 
    return 0;
 
  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }
 
  dump(text, trace_fp, (unsigned char *)data, size, config->trace_ascii);
  fclose(trace_fp);
  return 0;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
  RequestDataStream *pooh = (RequestDataStream *)userp;
  
  if(size*nmemb < 1)
    return 0;
 
  if(pooh->sizeleft) {
    *(char *)ptr = pooh->content[0]; /* copy one single byte */ 
    pooh->content++;                 /* advance pointer */ 
    pooh->sizeleft--;                /* less data left */ 
    return 1;                        /* we return 1 byte at a time! */ 
  }
 
  return 0;                          /* no more data left to deliver */ 
}
 
static size_t response_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  ResponseData *mem = (ResponseData *)userp;
 
  mem->data = realloc(mem->data, mem->size + realsize + 1);
  if(mem->data == NULL) {
    /* out of memory! */ 
    jsvlog_error("response_write_callback: not enough memory (realloc returned NULL)");
    return 0;
  }
 
  memcpy(&(mem->data[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->data[mem->size] = 0;
 
  return realsize;
}

/* JSON-data: [{"Id":1,"Name":"Kitchen","Status":"On"},
   {"Id":2,"Name":"Wall","Status":"On"},
   {"Id":3,"Name":"Window","Status":"Off"},
   {"Id":4,"Name":"Bookshelf","Status":"On"}] */
void post_current_status(double indoor_temprature, double outdoor_temprature, double indoor_humidity, int *statusArray, int number_of_devices)
{
  CURL *curl;
  CURLcode res;
  RequestDataStream dataStream;
  ResponseData response;

  response.data = (char *)calloc(1, sizeof(char));
  response.size = 0;

  //  int *statusArray = (int *)malloc(sizeof(int));
//  int number_of_devices = getDevicesStatus(statusArray);
  TRACE_CONFIG trace_config;
  trace_config.trace_ascii = 1; /* enable ascii tracing */ 
	
  int allocated_json = 500;
  char *json = (char *)calloc(allocated_json, sizeof(char));
  sprintf(json, "{\"IndoorTemprature\": %.2lf, \"IndoorHumidity\": %.2lf, \"OutdoorTemprature\": %.2lf, \"DeviceArray\":", 
	  indoor_temprature, indoor_humidity, outdoor_temprature);
    /*  mystrcpy(json, "");*/
  char deviceObject[200];
  int cur_json_len;
  int deviceObjectLen;
  for(int i = 0; i < number_of_devices; i++) {
    mymemset(deviceObject, 0, 200);
    int id = tdGetDeviceId(i);
    char *name = tdGetName( id );
    sprintf(deviceObject, "%c{\"Id\":%d,\"Name\":\"%s\",\"Status\":\"%s\"}",
	    i > 0 ? ',' : '[', 
	    id, name, statusArray[i] ? "true" : "false");
		
    deviceObjectLen = mystrlen(deviceObject);
    cur_json_len = mystrlen(json);
    if((deviceObjectLen + cur_json_len) > allocated_json) {
      allocated_json += (deviceObjectLen + 50); 
      json = (char *)realloc(json, allocated_json);
    }
    mystrcat(json, deviceObject);
    tdReleaseString(name);		
  }
  mystrcat(json, "]}");
  /*  jsvlog_debug(json); */
  curl = curl_easy_init();
  dataStream.content = json;
  dataStream.sizeleft = (long)mystrlen(json);
  jsvlog_debug("About to post %ld bytes of json %s", dataStream.sizeleft, json);
  if(curl) {
    jsvlog_debug("Setup call to %s", g_application.ws_client.url);

    
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
    curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &trace_config);
    /* the DEBUGFUNCTION has no effect until we enable VERBOSE 
       curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */
    curl_easy_setopt(curl, CURLOPT_URL, g_application.ws_client.url);

    /* Now specify we want to POST data */ 
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
 
    /* we want to use our own read function */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
 
    /* pointer to pass to our read function */ 
    curl_easy_setopt(curl, CURLOPT_READDATA, &dataStream);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    struct curl_slist *chunk = NULL; 
    chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    chunk = curl_slist_append(chunk, "Expect:");	
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);	
    chunk = curl_slist_append(chunk, "Content-Type: application/json");	
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);	

    jsvlog_debug("Send request");
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      jsvlog_info("%s", g_application.ws_client.url);
      jsvlog_info("%s", json);
      jsvlog_error("Failed to get data: %s", curl_easy_strerror(res));
    }

    
    /* skip first and last characters " */
    if(response.data) {
      jsvlog_debug("data: %s", response.data);
      if(mystrlen(response.data) > 2) {
	for(int i = 1; i < g_application.number_of_devices && *(response.data + i) && isdigit(*(response.data + i)); i++) {
	  jsvlog_debug("New status #%d: %d", i, *(response.data + i));
	  statusArray[i] = *(response.data + i) - '0';
	}
      }
    }
    else
      jsvlog_error("response.data was null");
    
    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);

  }
  else
    jsvlog_error("curl not set!");

  jsvlog_debug("post_current_status: About to free memory");
  if(response.data)
    free(response.data);
  else
    jsvlog_error("Failed to free response.data");
  
  if(json) 
    free(json);
  else
    jsvlog_error("Failed to free json");
		
  jsvlog_debug("post_current_status: jump out");
}

void get_changes_from_ws(int *device_array_status)
{
  CURL *curl;
  CURLcode res;
  ResponseData response;

  response.data = (char *)calloc(1, sizeof(char));
  response.size = 0;

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, g_application.ws_client.url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      jsvlog_error("Failed to get data: %s", curl_easy_strerror(res));
    }
    else {
      /*      
      printf("Response size: %d bytes\n", response.size);
      printf("Response data: %s\n", response.data);
      */
      /* skip first and last characters " */
      if(mystrlen(response.data) > 2) {
	for(int i = 0; i < g_application.number_of_devices; i++) {
	  device_array_status[i] = *(response.data + i + 1) - '0';
	}
      }
    }
    if(response.data) {
      free(response.data);
    }
    curl_easy_cleanup(curl);
  }
}


char *get_sunset_sunrise(double lat, double lng)
{
  CURL *curl;
  CURLcode res;
  ResponseData response;
  
  char *return_string = (char *)calloc(12, sizeof(char)); /* HH:MM;HH:MM */
  /*url + &longitude=15.123456&latitude=69.123456 */ 
  char *url = (char *)calloc(mystrlen(g_application.ws_client.sunUrl) + 50, sizeof(char));
  char *latlng = (char *)calloc(50, sizeof(char));
	
	
  response.data = (char *)calloc(1, sizeof(char));
  response.size = 0;

  curl = curl_easy_init();
  if(curl) {
	sprintf(latlng, WS_PARAM_FORMAT, lng, lat);
	mystrcpy(url, g_application.ws_client.sunUrl);
	mystrcat(url, latlng);
	
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "Error in WS-call: %s", curl_easy_strerror(res));
      jsvlog_error("Failed to get data: %s", curl_easy_strerror(res));
    }
    else {
      if(mystrlen(response.data) > 0) {
		mystrcpy(return_string, response.data);
      }
    }
    if(response.data) {
      free(response.data);
    }
    curl_easy_cleanup(curl);	
  }
  free(latlng);
  free(url);
  return return_string; 
}
