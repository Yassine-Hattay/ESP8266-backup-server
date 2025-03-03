/**
 * @file server_m.h
 * @author Hattay Yassine (hattayyassine519@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-03
 * 
 * 
 * 
 */

#ifndef SERVER_M_H   // Check if the macro is not defined
#define SERVER_M_H

#include <ESPAsyncWebServer.h>

extern char date[13];
extern bool end_server_b;
extern AsyncWebServer server;

void setup_server();

#endif
