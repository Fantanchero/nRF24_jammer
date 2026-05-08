#include "display_manager.h"

DisplayManager::DisplayManager()
    : display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {
}

bool DisplayManager::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        DEBUG_PRINTLN("SSD1306 allocation failed");
        return false;
    }
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);
    
    DEBUG_PRINTLN("Display initialized");
    return true;
}

void DisplayManager::powerOff() {
    display.ssd1306_command(0xAE);  // Display OFF
}

void DisplayManager::showSplash() {
    display.clearDisplay();
    
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(12, 8);
    display.println("BT Deauth");
    
    display.setTextSize(1);
    display.setCursor(28, 28);
    display.println("v2.0 Beta");
    
    display.setCursor(8, 40);
    display.println("Bluetooth Speaker");
    display.setCursor(16, 50);
    display.println("Disconnection PoC");
    
    display.display();
}

void DisplayManager::showMenu() {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== MAIN MENU ==");
    
    display.setCursor(0, 16);
    display.println("> Scan for Speakers");
    display.setCursor(2, 26);
    display.println("About");
    display.setCursor(2, 36);
    display.println("Settings");
    
    display.setCursor(0, 56);
    display.println("NEXT to navigate");
    
    display.display();
}

void DisplayManager::showScanning(uint8_t deviceCount, uint8_t elapsedSeconds) {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== SCANNING ==");
    
    display.setCursor(0, 16);
    display.print("Found: ");
    display.println(deviceCount);
    
    display.setCursor(0, 28);
    display.print("Time: ");
    display.print(elapsedSeconds);
    display.println("s");
    
    display.setCursor(0, 40);
    display.println("Searching for");
    display.println("Bluetooth speakers...");
    
    display.display();
}

void DisplayManager::showDeviceList(const char** devices, uint8_t count, uint8_t selectedIndex, const int8_t* rssiValues) {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== DEVICES ==");
    
    // Show selected device with indicator
    if (selectedIndex < count) {
        display.setCursor(0, 16);
        display.print("> ");
        display.println(devices[selectedIndex]);
        
        // Show RSSI
        display.setCursor(0, 28);
        display.print("RSSI: ");
        display.print(rssiValues[selectedIndex]);
        display.println(" dBm");
        
        drawSignalBars(0, 38, rssiValues[selectedIndex]);
    }
    
    // Show counter
    display.setCursor(0, 56);
    display.print("[");
    display.print(selectedIndex + 1);
    display.print("/");
    display.print(count);
    display.println("]");
    
    display.display();
}

void DisplayManager::showTargetConfirm(const char* name, const char* mac, int8_t rssi) {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== TARGET ==");
    
    display.setCursor(0, 14);
    display.print("Name: ");
    display.println(name);
    
    display.setCursor(0, 26);
    display.print("MAC: ");
    display.println(mac);
    
    display.setCursor(0, 38);
    display.print("RSSI: ");
    display.print(rssi);
    display.println(" dBm");
    
    display.setCursor(0, 56);
    display.println("OK to start attack");
    
    display.display();
}

void DisplayManager::showAttacking(const char* targetName, uint32_t packetCount, uint8_t progress) {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== ATTACKING ==");
    
    display.setCursor(0, 14);
    display.println(targetName);
    
    display.setCursor(0, 26);
    display.print("Packets: ");
    display.println(packetCount);
    
    display.setCursor(0, 38);
    drawProgressBar(0, 40, 128, 8, progress);
    
    display.setCursor(0, 56);
    display.print(progress);
    display.println("%");
    
    display.display();
}

void DisplayManager::showSuccess(const char* targetName, uint32_t packetCount) {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== SUCCESS ==");
    
    display.setCursor(0, 16);
    display.println("Disconnection");
    display.println("detected!");
    
    display.setCursor(0, 38);
    display.print("Target: ");
    display.println(targetName);
    
    display.setCursor(0, 50);
    display.print("Packets: ");
    display.println(packetCount);
    
    display.display();
}

void DisplayManager::showError(const char* errorMsg) {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("== ERROR ==");
    
    display.setCursor(0, 16);
    display.println(errorMsg);
    
    display.setCursor(0, 56);
    display.println("Press OK to continue");
    
    display.display();
}

void DisplayManager::drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t percentage) {
    if (percentage > 100) percentage = 100;
    
    // Draw border
    display.drawRect(x, y, width, height, SSD1306_WHITE);
    
    // Draw fill
    uint16_t fillWidth = (width - 2) * percentage / 100;
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
}

void DisplayManager::drawSignalBars(int16_t x, int16_t y, int8_t rssi) {
    uint8_t bars = rssiToBars(rssi);
    
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t barHeight = 2 + (i * 2);
        uint8_t color = (i < bars) ? SSD1306_WHITE : SSD1306_BLACK;
        
        display.fillRect(x + i * 6, y + 8 - barHeight, 4, barHeight, color);
        display.drawRect(x + i * 6, y + 8 - barHeight, 4, barHeight, SSD1306_WHITE);
    }
}

void DisplayManager::update() {
    display.display();
}
