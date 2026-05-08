#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <string.h>
#include "config.h"

/**
 * @brief Convert MAC address bytes to string format
 * @param mac 6-byte MAC address
 * @param buffer Output buffer
 * @param bufferSize Size of output buffer
 */
inline void macToString(const uint8_t* mac, char* buffer, uint16_t bufferSize) {
    if (!mac || !buffer || bufferSize < 18) return;
    snprintf(buffer, bufferSize, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * @brief Convert RSSI to signal strength percentage
 * @param rssi RSSI value in dBm (typically -30 to -100)
 * @return Percentage 0-100
 */
inline uint8_t rssiToPercentage(int8_t rssi) {
    if (rssi >= -30) return 100;
    if (rssi <= -100) return 0;
    return (rssi + 100) * 2;  // Scale -100 to -30 to 0-100
}

/**
 * @brief Convert RSSI to signal strength bars (1-5)
 * @param rssi RSSI value in dBm
 * @return Signal bars 1-5
 */
inline uint8_t rssiToBars(int8_t rssi) {
    uint8_t percentage = rssiToPercentage(rssi);
    if (percentage >= 80) return 5;
    if (percentage >= 60) return 4;
    if (percentage >= 40) return 3;
    if (percentage >= 20) return 2;
    return 1;
}

/**
 * @brief Check if a device name contains audio keywords
 * @param name Device name to check
 * @return true if name matches audio device keywords
 */
inline bool isAudioDeviceName(const char* name) {
    if (!name || strlen(name) == 0) return false;
    
    for (int i = 0; i < AUDIO_KEYWORDS_COUNT; i++) {
        if (strstr(name, AUDIO_KEYWORDS[i]) != NULL) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Generate pseudo-random hopping sequence
 * @param sequence Output buffer
 * @param length Number of channels to generate
 */
inline void generateHoppingSequence(uint8_t* sequence, uint8_t length) {
    if (!sequence || length == 0) return;
    
    // Generate random channel sequence (0-78 for Bluetooth)
    for (int i = 0; i < length; i++) {
        sequence[i] = random(0, 79);  // Bluetooth uses channels 0-78
    }
}

/**
 * @brief Safe string copy with length checking
 * @param dst Destination buffer
 * @param src Source string
 * @param maxLen Maximum length to copy
 */
inline void safeCopyString(char* dst, const char* src, uint16_t maxLen) {
    if (!dst || !src) return;
    strncpy(dst, src, maxLen - 1);
    dst[maxLen - 1] = '\0';
}

/**
 * @brief Get current time in milliseconds
 */
inline uint32_t getCurrentTime() {
    return millis();
}

/**
 * @brief Calculate elapsed time in milliseconds
 */
inline uint32_t getElapsedTime(uint32_t startTime) {
    return getCurrentTime() - startTime;
}

#endif
