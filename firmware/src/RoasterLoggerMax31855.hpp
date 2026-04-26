#pragma once

#include "RoasterLogger.hpp"
// #include <max31855.hpp>

class RoasterLoggerMax31855 : public RoasterLogger {
public:
    RoasterLoggerMax31855(uint16_t logBufferSize, uint8_t spiCsPin);
    bool save() override;// one shot save
protected:
    // Max31855 thermalBeanTemperature_;
};
