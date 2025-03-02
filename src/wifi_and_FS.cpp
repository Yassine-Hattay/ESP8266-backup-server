#include "wifi_and_FS.h"
#include "server_m.h"

WiFiUDP udp;
bool connected_wifi;
// Flag to check if transmission is complete
unsigned long startTime = 0;           // Declare the startTime variable
bool moreFiles;
unsigned long lastTime2 = 0;

bool loadFileToLittleFS(const String &sourcePath, const String &destPath)
{
  // Open SD file
  File sdFile = SD.open(sourcePath, "r");
  if (!sdFile)
  {
    Serial.println("Failed to open source file on SD card: " + sourcePath);
    return false;
  }

  // Create or open the file in LittleFS
  File littlefsFile = LittleFS.open(destPath, "w");
  if (!littlefsFile)
  {
    Serial.println("Failed to open destination file on LittleFS: " + destPath);
    sdFile.close();
    return false;
  }

  // Copy file from SD to LittleFS in chunks
  byte buffer[2048];
  while (sdFile.available())
  {
    size_t bytesRead = sdFile.read(buffer, sizeof(buffer));
    littlefsFile.write(buffer, bytesRead);
    Serial.printf("Free RAM: %u bytes\n", ESP.getFreeHeap());
  }

  sdFile.close();
  littlefsFile.close();
  Serial.println("File successfully copied to LittleFS: " + destPath);
  return true;
}



void printMemoryAndFileSystemStats()
{
  Serial.println("Memory and LittleFS Statistics:");
  // LittleFS Flash Information
  if (LittleFS.info(fs_info))
  {
    Serial.printf("Total LittleFS Space: %u bytes\n", fs_info.totalBytes);
    Serial.printf("Used LittleFS Space: %u bytes\n", fs_info.usedBytes);
    Serial.printf("Free LittleFS Space: %u bytes\n", fs_info.totalBytes - fs_info.usedBytes);
  }
  else
  {
    Serial.println("Failed to retrieve LittleFS information.");
  }
  Serial.printf("Free RAM: %u bytes\n", ESP.getFreeHeap());
}

uint8_t getWiFiChannel(const char *ssid)
{
  if (uint8_t n = WiFi.scanNetworks())
  {
    for (uint8_t i = 0; i < n; i++)
    {
      if (!strcmp(ssid, WiFi.SSID(i).c_str()))
      {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

bool connectToWiFi(const char *ssid, const char *password, unsigned long timeout)
{

  WiFi.begin(ssid, password); // Start connecting to WiFi
  Serial.print("Connecting to WiFi");

  unsigned long startTime = millis(); // Record the start time

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    // Check if the timeout has been exceeded
    if (millis() - startTime >= timeout)
    {
      Serial.println("\nWiFi connection timeout.");
      return false; // Connection failed within the timeout
    }
  }

  Serial.println("\nWiFi connected!");

  setup_server();

  return true; // Successfully connected
}
