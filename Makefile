CFLAGS=-c -Wall -std=c99 -pthread -g -D_POSIX_C_SOURCE=1
LDFLAGS=-ltelldus-core -lcurl -pthread
CC=gcc

TARGET=jsvtimerd
SOURCES=jsvtimerd.c jsvtellstick.c mystring.c configuration.c logging.c ws_client.c device_config_list.c device.c
SOURCES_TST=${SOURCES} unittests.c
HEADERS=jsvtellstick.h mytypes.h mystring.h configuration.h logging.h ws_client.h device_config_list.h device.h
OBJECTS=$(SOURCES:.c=.o)
OBJETCS_TST=$(SOURCES_TST:.c=.o)
SRCTEST=unittests.c configuration.c mystring.c device_config_list.c device.c jsvtellstick.c logging.c ws_client.c
OBJTEST=$(SRCTEST:.c=.o)
TESTTARGET=unittests

all: ${SOURCES} ${HEADERS} ${TARGET}

${TARGET}: ${OBJECTS}
	${CC} -o ${TARGET} ${OBJECTS} ${LDFLAGS}

testa: ${OBJTEST}
	${CC} -o ${TESTTARGET} ${OBJTEST} ${LDFLAGS}

.c.o:	${HEADERS}
	${CC} ${CFLAGS} $< -o $@

cleanobj: 
	rm -f ${OBJECTS}

clean:
	rm -f ${TARGET} ${OBJECTS} *~
