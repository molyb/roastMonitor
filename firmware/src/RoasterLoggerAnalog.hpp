#pragma once

#include "RoasterLogger.hpp"

class RoasterLoggerAnalog : public RoasterLogger {
public:
    RoasterLoggerAnalog(uint16_t logBufferSize, uint8_t adcPin);
    bool save() override;// one shot save
protected:
    // Max31855 thermalBeanTemperature_;
};
