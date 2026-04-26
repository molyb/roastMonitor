#include "RoasterLogger.hpp"
#include <Arduino.h>

RoasterLogger::RoasterLogger(uint16_t logBufferSize)
    : logBufferSize_(logBufferSize)
    , isLogging_(false)
{
}

bool RoasterLogger::startLogging(uint32_t interval_ms)
{
    if (!isLogging_) {
        ticker_.attach_ms(interval_ms, [this]() -> void { this->save(); });
    }
    isLogging_ = true;
    return true;
}

bool RoasterLogger::stopLogging()
{
    if (isLogging_) {
        ticker_.detach();
    }
    isLogging_ = false;
    return true;
}

void RoasterLogger::reset()
{
    std::lock_guard<std::shared_mutex> lock(logMutex_);
    log_.clear();
}

void RoasterLogger::appendLog(const Temperature& temperature)
{
    std::lock_guard<std::shared_mutex> lock(logMutex_);
    while (logBufferSize_ <= log_.size()) {
        log_.pop_front();
    }
    log_.push_back(temperature);
}

std::deque<RoasterLogger::Temperature> RoasterLogger::getLog() const
{
    std::shared_lock<std::shared_mutex> lock(logMutex_);
    return log_;
}

RoasterLogger::Temperature RoasterLogger::getLatest() const
{
    std::shared_lock<std::shared_mutex> lock(logMutex_);
    if (log_.size() == 0) {
        return RoasterLogger::Temperature();
    }
    return log_.back();
}
