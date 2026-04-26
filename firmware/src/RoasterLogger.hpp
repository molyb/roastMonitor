#pragma once

#include <Ticker.h>
#include <deque>
#include <shared_mutex>
#include <sys/time.h>

class RoasterLogger {
public:
    RoasterLogger(uint16_t logBufferSize);
    bool startLogging(uint32_t interval_ms);
    bool stopLogging();
    virtual bool save() {return true;}; // one shot save
    void reset();

    struct Temperature {
        Temperature()
            : time({ time_t(NULL), 0 })
            , beanTemperature(0)
            , environmentTemperature(0) {};
        Temperature(timeval time, double bean, double env)
            : time(time)
            , beanTemperature(bean)
            , environmentTemperature(env) {};
        timeval time;
        double beanTemperature;
        double environmentTemperature;
    };

    std::deque<Temperature> getLog() const;
    Temperature getLatest() const;

private:
    mutable std::shared_mutex logMutex_;
    std::deque<Temperature> log_;
    uint16_t logBufferSize_;
    Ticker ticker_;
    bool isLogging_;
};
