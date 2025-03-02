#include "wifi_and_FS.h"
#include "esp_now_m.h"
#include "server_m.h"

AsyncWebServer server(80);

// WiFi credentials
const char ssid[] = "Orange-066C";
const char password[] = "GMA6ABLMG87";

// NTP settings
NTPClient timeClient(udp, "pool.ntp.org", 3600, 3600000); // Offset: UTC+1 (3600 seconds)

void setup()
{

  Serial.begin(115200);

  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.mode(WIFI_AP);

  connected_wifi = connectToWiFi(ssid, password, 10000);

  if (connected_wifi == false)
  {
    server.end();                     // Stop the server
    WiFi.disconnect();                // Disconnect WiFi
    WiFi.mode(WIFI_STA);              // Set WiFi mode to Station
    wifi_set_channel(ESPNOW_CHANNEL); // Set the WiFi channel
  }

  InitESPNow();
  esp_now_register_recv_cb(OnDataRecv);

  // Initialize SD card
  while (!SD.begin(D8))
  {
    Serial.println("SD card initialization failed!");
    delay(100);
    Serial.println("Retrying!");
    SD.end();
  }

  Serial.println("SD card initialized successfully.");

  timeClient.begin();
  // Start NTP client to fetch time
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  setTime(epochTime); // Sets time for TimeLib functions

  timeClient.end();

  Serial.println("Async Web server initialized.");
  if (LittleFS.format())
  {
    Serial.println("LittleFS formatted successfully!");
  }
  else
  {
    Serial.println("Failed to format LittleFS!");
  }
  printMemoryAndFileSystemStats();
}

void loop()
{
  if (end_server_b)
  {
    WiFi.forceSleepBegin(); // Disconnect WiF
    Serial.print("\n Sleeping .\n");
    wifi_fpm_do_sleep(2000000);

    Serial.print("Woke up .\n");
    WiFi.mode(WIFI_STA);
    int lastTime = millis();
    while (int(millis()) - lastTime < 2000)
    {
      yield();
    }
  }
  else if (millis() - lastTime2 > 900000 && !end_server_b)
  {
    server.end();                     // Stop the server
    WiFi.disconnect();                // Disconnect WiFi
    WiFi.mode(WIFI_STA);              // Set WiFi mode to Station
    wifi_set_channel(ESPNOW_CHANNEL); // Set the WiFi channel

    WiFi.forceSleepBegin();
    delay(1); // Allow some time for WiFi to go into sleep

    // Set ESP8266 to light sleep
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); // Set to light sleep mode
    wifi_fpm_open();                        // Open light sleep mod

    end_server_b = true;
  }
}
