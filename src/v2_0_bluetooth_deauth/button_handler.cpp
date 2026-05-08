#include "button_handler.h"

ButtonHandler::ButtonHandler()
    : button1State(false),
      button2State(false),
      button1PressTime(0),
      button2PressTime(0),
      button1Pressed(false),
      button2Pressed(false) {
}

bool ButtonHandler::begin() {
    pinMode(PIN_BUTTON_1, INPUT_PULLUP);
    pinMode(PIN_BUTTON_2, INPUT_PULLUP);
    
    DEBUG_PRINTLN("Button handler initialized");
    return true;
}

void ButtonHandler::update() {
    // Read button states (active LOW with pull-up)
    bool btn1Read = !digitalRead(PIN_BUTTON_1);
    bool btn2Read = !digitalRead(PIN_BUTTON_2);
    
    // Button 1 debounce and detection
    if (btn1Read && !button1State) {
        // Falling edge - button pressed
        button1PressTime = millis();
        button1State = true;
    } else if (!btn1Read && button1State) {
        // Rising edge - button released
        button1State = false;
        uint32_t pressDuration = millis() - button1PressTime;
        
        if (pressDuration > BUTTON_LONG_PRESS_TIME) {
            button1Pressed = BUTTON_LONG_PRESS;
        } else if (pressDuration > BUTTON_DEBOUNCE_TIME) {
            button1Pressed = BUTTON_SHORT_PRESS;
        }
    }
    
    // Button 2 debounce and detection
    if (btn2Read && !button2State) {
        // Falling edge - button pressed
        button2PressTime = millis();
        button2State = true;
    } else if (!btn2Read && button2State) {
        // Rising edge - button released
        button2State = false;
        uint32_t pressDuration = millis() - button2PressTime;
        
        if (pressDuration > BUTTON_LONG_PRESS_TIME) {
            button2Pressed = BUTTON_LONG_PRESS;
        } else if (pressDuration > BUTTON_DEBOUNCE_TIME) {
            button2Pressed = BUTTON_SHORT_PRESS;
        }
    }
}

uint8_t ButtonHandler::getButton1Event() {
    uint8_t event = button1Pressed;
    button1Pressed = BUTTON_NO_PRESS;
    return event;
}

uint8_t ButtonHandler::getButton2Event() {
    uint8_t event = button2Pressed;
    button2Pressed = BUTTON_NO_PRESS;
    return event;
}

bool ButtonHandler::isButton1Pressed() const {
    return button1State;
}

bool ButtonHandler::isButton2Pressed() const {
    return button2State;
}
