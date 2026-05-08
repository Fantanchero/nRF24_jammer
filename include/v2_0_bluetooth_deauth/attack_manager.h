#ifndef ATTACK_MANAGER_H
#define ATTACK_MANAGER_H

#include <stdint.h>
#include "config.h"
#include "button_handler.h"
#include "display_manager.h"
#include "bt_sniffer.h"
#include "nrf24_injector.h"

/**
 * @brief Main application state machine
 * Orchestrates BLE scanning, target selection, and nRF24 attack
 */
class AttackManager {
public:
    AttackManager();
    
    /**
     * @brief Initialize all subsystems
     * @return true if all systems initialized successfully
     */
    bool begin();
    
    /**
     * @brief Main update loop (call frequently)
     */
    void update();
    
    /**
     * @brief Shutdown all subsystems
     */
    void shutdown();

private:
    enum class State {
        SPLASH,
        MENU,
        SCANNING,
        DEVICE_SELECT,
        TARGET_CONFIRM,
        ATTACKING,
        ERROR,
        SUCCESS
    };
    
    State currentState;
    State nextState;
    
    ButtonHandler buttons;
    DisplayManager display;
    BTSniffer btSniffer;
    NRF24Injector nrf24;
    
    // Scan state
    uint32_t scanStartTime;
    uint32_t lastScanUpdateTime;
    
    // Target state
    uint8_t selectedDeviceIndex;
    uint8_t targetMAC[6];
    char targetName[32];
    int8_t targetRSSI;
    
    // Attack state
    uint32_t attackStartTime;
    uint32_t lastPacketSendTime;
    uint32_t totalPacketsSent;
    uint8_t attackProgress;
    bool attackRunning;
    
    // Error handling
    char errorMessage[64];
    
    // ============================================
    // State Transition Methods
    // ============================================
    
    /**
     * @brief Handle splash screen state
     */
    void handleSplash();
    
    /**
     * @brief Handle main menu state
     */
    void handleMenu();
    
    /**
     * @brief Handle BLE scanning state
     */
    void handleScanning();
    
    /**
     * @brief Handle device selection state
     */
    void handleDeviceSelect();
    
    /**
     * @brief Handle target confirmation state
     */
    void handleTargetConfirm();
    
    /**
     * @brief Handle attack state
     */
    void handleAttacking();
    
    /**
     * @brief Handle error state
     */
    void handleError();
    
    /**
     * @brief Handle success state
     */
    void handleSuccess();
    
    // ============================================
    // Helper Methods
    // ============================================
    
    /**
     * @brief Transition to new state
     */
    void setState(State newState);
    
    /**
     * @brief Set error message and go to error state
     */
    void setError(const char* msg);
};

#endif
