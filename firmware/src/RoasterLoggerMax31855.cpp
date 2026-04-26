#include "RoasterLoggerMax31855.hpp"

RoasterLoggerMax31855::RoasterLoggerMax31855(uint16_t logBufferSize, uint8_t spiCsPin):
    RoasterLogger(logBufferSize)
{
}



bool RoasterLoggerMax31855::save()
{
    // timeval tv;
    // gettimeofday(&tv, NULL);
    // std::lock_guard<std::shared_mutex> lock(logMutex_);
    // // 新規要素を追加するため、バッファ上限 - 1 の要素数となるよう古いデータをpop
    // while (logBufferSize_ <= log_.size()) {
    //     log_.pop_front();
    // }
    // float temperature = thermalBeanTemperature_.read();
    // if (isnan(temperature)) {
    //     Serial.println("Failed to read temperature from MAX31855");
    //     return false;
    // }
    // log_.emplace_back(tv, temperature, 0);
    return true;
}
