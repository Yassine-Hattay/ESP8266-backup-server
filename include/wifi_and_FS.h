#ifndef WIFI_AND_FS_H
#define WIFI_AND_FS_H

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SD.h>
#include "LittleFS.h"
#include <NTPClient.h>
#include <TimeLib.h>
#include <espnow.h>

extern File dir;
extern File file;
extern File root;
extern FSInfo fs_info;
extern bool connected_wifi;
extern unsigned long startTime ;           
extern bool moreFiles;
extern unsigned long lastTime2 ;
extern WiFiUDP udp;

bool connectToWiFi(const char *ssid, const char *password, unsigned long timeout);
void printMemoryAndFileSystemStats();
bool loadFileToLittleFS(const String &sourcePath, const String &destPath);

#endif
