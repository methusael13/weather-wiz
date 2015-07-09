
# Makefile for Weather-Wiz
# @author: Methusael Murmu

DEP_GTK=gtk+-2.0
DEP_JSON=jansson
DEP_CURL=libcurl
DEP_APP_INDICATOR=appindicator-0.1
DEP_PACKAGE=$(DEP_GTK) $(DEP_JSON) $(DEP_CURL) $(DEP_APP_INDICATOR)

VPATH=src
INCLUDES=-I include

CC=gcc
CFLAGS=-Wall `pkg-config --cflags $(DEP_PACKAGE)`
LIBS=`pkg-config --libs $(DEP_PACKAGE)` -pthread
COMPILE=$(CC) $(INCLUDES) $(CFLAGS) -c
OUTPUT_OPTION=-o $@

BIN_DIR=bin
TEST_DIR=tests
SOURCES= \
	main.c weather_backend.c \
	timer_thread.c json_util.c \
	util.c
OBJECTS=$(subst .c,.o,$(SOURCES))
BIN=weather-wiz
MKDIR=mkdir -p
RM=rm -rf

# Setup for bin
define perform_post_build
	@if [ ! -d "$BIN_DIR" ]; then \
		$(MKDIR) $(BIN_DIR); \
	fi; \
	mv $(BIN) $(BIN_DIR)/.; \
	$(RM) *.o
endef

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $^ -o $@ $(LIBS)
	$(setup-bin)
	$(perform_post_build)

main.o: main.c
	$(COMPILE) $<
	
weather_backend.o: weather_backend.c
	$(COMPILE) $<

timer_thread.o: timer_thread.c
	$(COMPILE) $<

json_util.o: json_util.c
	$(COMPILE) $<

util.o: util.c
	$(COMPILE) $<

.PHONY: clean
clean:
	rm *.o
