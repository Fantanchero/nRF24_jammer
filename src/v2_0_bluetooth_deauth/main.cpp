#include <Arduino.h>
#include "config.h"
#include "attack_manager.h"

// Global attack manager
static AttackManager attackManager;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(100);
    
    DEBUG_PRINTLN("\n\n==========================================");
    DEBUG_PRINTLN("  BT Deauth v2.0 - Bluetooth Speaker PoC");
    DEBUG_PRINTLN("==========================================\n");
    
    // Initialize attack manager
    if (!attackManager.begin()) {
        DEBUG_PRINTLN("FATAL: Failed to initialize attack manager");
        while(1) delay(1000);
    }
    
    DEBUG_PRINTLN("System initialized successfully!");
}

void loop() {
    // Main application loop
    attackManager.update();
    
    // Brief yield to prevent watchdog
    delay(10);
}
