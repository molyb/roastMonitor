#pragma once

#include "RoasterLogger.hpp"

class RoasterLoggerAnalog : public RoasterLogger {
public:
    struct Pins {
        uint8_t environment;
        uint8_t bean;
    };

    RoasterLoggerAnalog(uint16_t logBufferSize, Pins pins);
    bool save() override; // one shot save

private:
    Pins pins_;
};
