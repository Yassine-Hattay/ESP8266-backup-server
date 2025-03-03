/**
 * @file esp_now_m.cpp
 * @author Hattay Yassine (hattayyassine519@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-03
 * 
 *
 * 
 */

#include "esp_now_m.h"
#include "wifi_and_FS.h"

uint16_t currentTransmitCurrentPosition = 0;
uint16_t currentTransmitTotalPackages = 0;
bool fileReceivingStarted = false;
bool fileTransmissionComplete = false;
File dir;
File file;
File root;
FSInfo fs_info;

/**
 * @brief this initializes esp now 
 *
 * @param parameter
 */

void InitESPNow()
{
    if (esp_now_init() == 0)
    {
        Serial.println("ESPNow Initialized Successfully");
    }
    else
    {
        Serial.println("ESPNow Initialization Failed. Restarting...");
        ESP.restart();
    }
}

/**
 * @brief this function excutes when data is recived and saves it to SD card . 
 *
 * @param parameter
 */

void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len)
{
    static byte buffer[BUFFER_SIZE]; // Use larger buffer for faster data handling
    int bufferIndex = 0;

    String currentDate = String(year()) + "-" + String(month()) + "-" + String(day());
    String folderPath = "/photos/" + currentDate;

    switch (*data++)
    {
    case 0x01: // Start of file transmission
    {
        if (!SD.exists(folderPath))
        {
            if (SD.mkdir(folderPath))
            {
                Serial.println("Folder created: " + folderPath);
            }
            else
            {
                Serial.println("Failed to create folder: " + folderPath);
            }
        }
        else
        {
            Serial.println("Folder already exists: " + folderPath);
        }

        currentTransmitCurrentPosition = 0;
        currentTransmitTotalPackages = (*data++) << 8 | *data;

        // Get current time in hours and minutes for file naming
        String currentTime = String(hour()) + "-" + String(minute()) + "-" + String(second());
        // Open the file for writing with the current time in the filename
        file = SD.open(folderPath + "/" + currentTime + ".jpg", "w");
        if (!file)
        {
            Serial.println("Error opening file for writing!");
            return;
        }

        // Stop the server when file reception starts
        Serial.println("Server stopped to receive file.");

        // Record the start time of the transfer
        break;
    }
    case 0x02: // Data pack
        if (currentTransmitCurrentPosition == 0)
        {
            startTime = millis();
        }

        byte highByte = *data++;
        byte lowByte = *data++;
        currentTransmitCurrentPosition = (highByte << 8) | lowByte;

        // Store data in the buffer
        for (int i = 0; i < (data_len - 3); i++)
        {
            buffer[bufferIndex++] = *data++; // Fill buffer with incoming data
            if (bufferIndex >= BUFFER_SIZE)
            {
                file.write(buffer, bufferIndex);
                Serial.printf("\n Buffer index: %d \n", bufferIndex); // Write buffer to file once full
                bufferIndex = 0;                                      // Reset buffer index
            }
        }

        // Handle any remaining data in the buffer after the loop
        if (bufferIndex > 0)
        {
            file.write(buffer, bufferIndex);
            bufferIndex = 0; // Reset buffer
        }

        Serial.printf("Received packet %d of %d\n", currentTransmitCurrentPosition, currentTransmitTotalPackages);
        break;
    }

    if (currentTransmitCurrentPosition == currentTransmitTotalPackages)
    {
        Serial.printf("File transfer complete! Size: %d bytes\n", file.size());

        // Calculate the time taken for the transfer
        int transferTime = millis() - startTime;
        int transferTimeInSeconds = transferTime / 1000;                           // Convert milliseconds to seconds
        Serial.printf("Total transfer time: %d seconds\n", transferTimeInSeconds); // Print total time in seconds

        file.close(); // Close the file once the transfer is complete
        fileReceivingStarted = false;
        fileTransmissionComplete = false; // Reset flags for next transfer
    }
}