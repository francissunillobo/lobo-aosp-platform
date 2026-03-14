package com.lobo.platform.fansettings;

interface IFanSettingsService {
    int getMaxTemperature();
    void setMaxTemperature(int tempCelsius);
    int getMinTemperature();
    void setMinTemperature(int tempCelsius);
    boolean isAutoMode();
    void setAutoMode(boolean enabled);
}
