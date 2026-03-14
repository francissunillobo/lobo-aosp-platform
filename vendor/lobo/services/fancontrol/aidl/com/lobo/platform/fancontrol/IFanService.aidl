package com.lobo.platform.fancontrol;

interface IFanService {
    int getDutyCycle();
    void setDutyCycle(int dutyCycle);
    float getTemperature();
    boolean isRunning();
}
