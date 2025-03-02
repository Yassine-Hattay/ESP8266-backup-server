#ifndef SERVER_M_H   // Check if the macro is not defined
#define SERVER_M_H

#include <ESPAsyncWebServer.h>

extern char date[13];
extern bool end_server_b;
extern AsyncWebServer server;

void setup_server();

#endif
