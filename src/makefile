
# Makefile for Weather-Wiz
# @author: Methusael Murmu

DEP_GTK=gtk+-2.0
DEP_JSON=jansson
DEP_CURL=libcurl
DEP_APP_INDICATOR=appindicator-0.1
DEP_PACKAGE=$(DEP_GTK) $(DEP_JSON) $(DEP_CURL) $(DEP_APP_INDICATOR)

CC=gcc
CFLAGS=-Wall `pkg-config --cflags $(DEP_PACKAGE)` -c
LIBS=`pkg-config --libs $(DEP_PACKAGE)` -pthread

BIN_DIR=../bin

OBJ=util.o json_util.o timer_thread.o weather_backend.o main.o
BIN=$(BIN_DIR)/weather-wiz

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBS) && rm -rf *.o

main.o: main.c
	$(CC) $(CFLAGS) main.c
	
weather_backend.o: weather_backend.c
	$(CC) $(CFLAGS) weather_backend.c

timer_thread.o: timer_thread.c
	$(CC) $(CFLAGS) timer_thread.c

json_util.o: json_util.c
	$(CC) $(CFLAGS) json_util.c

util.o: util.c
	$(CC) $(CFLAGS) util.c

clean:
	rm *.o $(BIN)
