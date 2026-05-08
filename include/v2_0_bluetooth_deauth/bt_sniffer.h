#ifndef BT_SNIFFER_H
#define BT_SNIFFER_H

#include <NimBLEDevice.h>
#include "config.h"
#include "utils.h"

/**
 * @brief BLE device descriptor for discovered speakers
 */
struct BTDevice {
    uint8_t mac[6];
    char name[TARGET_NAME_MAX];
    int8_t rssi;
    uint16_t appearance;
    uint32_t lastSeen;
};

/**
 * @brief Bluetooth speaker scanner using NimBLE
 * Detects audio devices via appearance codes and device names
 */
class BTSniffer {
public:
    BTSniffer();
    
    /**
     * @brief Initialize BLE scanner
     * @return true on success
     */
    bool begin();
    
    /**
     * @brief Shutdown BLE scanner
     */
    void end();
    
    /**
     * @brief Start scanning for BLE devices
     * @param durationSeconds Scan duration in seconds
     */
    void startScan(uint8_t durationSeconds);
    
    /**
     * @brief Stop ongoing scan
     */
    void stopScan();
    
    /**
     * @brief Get number of discovered devices
     * @return Device count
     */
    uint8_t getDiscoveredCount() const;
    
    /**
     * @brief Get device at index
     * @param index Device index
     * @return Pointer to BTDevice or NULL if invalid
     */
    BTDevice* getDevice(uint8_t index);
    
    /**
     * @brief Get device by MAC address
     * @param mac MAC address to search
     * @return Pointer to BTDevice or NULL if not found
     */
    BTDevice* getDeviceByMAC(const uint8_t* mac);
    
    /**
     * @brief Clear all discovered devices
     */
    void clearDevices();

private:
    BTDevice devices[MAX_TARGETS];
    uint8_t deviceCount;
    
    BLEScan* pBLEScan;
    uint32_t scanStartTime;
    bool scanning;
    
    /**
     * @brief Check if device is an audio device
     * @param appearance BLE appearance code
     * @param name Device name
     * @return true if appears to be audio device
     */
    bool isAudioDevice(uint16_t appearance, const char* name);
    
    /**
     * @brief Check if device is already in list
     * @param mac MAC address to check
     * @return Index of device or MAX_TARGETS if not found
     */
    uint8_t findDeviceIndex(const uint8_t* mac);
    
    /**
     * @brief Add or update device in list
     * @param mac Device MAC
     * @param name Device name
     * @param rssi Signal strength
     * @param appearance BLE appearance code
     */
    void addOrUpdateDevice(const uint8_t* mac, const char* name, int8_t rssi, uint16_t appearance);
};

#endif
