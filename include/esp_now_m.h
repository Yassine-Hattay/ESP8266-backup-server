#ifndef ESP_NOW_M_H   // Check if the macro is not defined
#define ESP_NOW_M_H

#include <stdint.h>

#define BUFFER_SIZE 4096
#define ESPNOW_CHANNEL 13

extern uint16_t currentTransmitCurrentPosition;
extern uint16_t currentTransmitTotalPackages ;
extern bool fileReceivingStarted ;     // Flag to check if receiving has started
extern bool fileTransmissionComplete ;

void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len);
void InitESPNow();

#endif  // End of include guard
