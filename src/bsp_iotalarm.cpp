#include "main.h"



bool bsp_hwInit(hwConfigType *s_config) { 
    // Initialize IO
    pinMode(s_config->ioStatusLED, OUTPUT);
    pinMode(s_config->ioSecurity, INPUT_PULLUP);
    pinMode(s_config->ioHelp, INPUT_PULLUP);
    pinMode(s_config->ioWifiReset, INPUT_PULLUP);

    // Initialize UART
    Serial.begin(s_config->baudrate);
    return true;
}

void bsp_toggleStatusLED(hwConfigType *s_config) {
    digitalWrite(s_config->ioStatusLED, !digitalRead(s_config->ioStatusLED));
}

bool bsp_isSecurityOccur(hwConfigType *s_config) {
    return !digitalRead(s_config->ioSecurity);
}

bool bsp_isHelpOccur(hwConfigType *s_config) {
    return !digitalRead(s_config->ioHelp);
}

bool bsp_isWifiResetButtonPressed(hwConfigType *s_config) {
    return !digitalRead(s_config->ioWifiReset);
}
