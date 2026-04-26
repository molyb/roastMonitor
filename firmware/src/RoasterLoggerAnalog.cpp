#include "RoasterLoggerAnalog.hpp"

#include <Arduino.h>
#include <math.h>
#include <sys/time.h>

namespace {
constexpr uint8_t kAdcResolutionBits = 12;
constexpr size_t kAdcSamples = 8;

constexpr double kMcp9701aOffsetVolts = 0.4;
constexpr double kMcp9701aVoltsPerDegreeC = 0.0195;

constexpr double kThermocoupleAmplifierGain = 100000.0 / (100.0 + 270.0);
constexpr double kThermocoupleOffsetVolts = 3.0 * 100.0 / (1000.0 + 100.0);
constexpr double kThermocoupleOffsetMilliVolts = kThermocoupleOffsetVolts * 1000.0;

double evaluatePolynomial(const double* coefficients, size_t coefficientCount, double x)
{
    double y = 0.0;
    double xPow = 1.0;
    for (size_t i = 0; i < coefficientCount; ++i) {
        y += coefficients[i] * xPow;
        xPow *= x;
    }
    return y;
}

double convertKTypeTempToMv(double temperatureC)
{
    if (temperatureC < -270.0) {
        temperatureC = -270.0;
    } else if (1372.0 < temperatureC) {
        temperatureC = 1372.0;
    }

    if (temperatureC < 0.0) {
        static const double c[] = {
            0.000000000000E+00,
            0.394501280250E-01,
            0.236223735980E-04,
            -0.328589067840E-06,
            -0.499048287770E-08,
            -0.675090591730E-10,
            -0.574103274280E-12,
            -0.310888728940E-14,
            -0.104516093650E-16,
            -0.198892668780E-19,
            -0.163226974860E-22
        };
        return evaluatePolynomial(c, sizeof(c) / sizeof(c[0]), temperatureC);
    }

    static const double c[] = {
        -0.176004136860E-01,
        0.389212049750E-01,
        0.185587700320E-04,
        -0.994575928740E-07,
        0.318409457190E-09,
        -0.560728448890E-12,
        0.560750590590E-15,
        -0.320207200030E-18,
        0.971511471520E-22,
        -0.121047212750E-25
    };
    constexpr double a0 = 0.118597600000E+00;
    constexpr double a1 = -0.118343200000E-03;
    constexpr double a2 = 0.126968600000E+03;
    return evaluatePolynomial(c, sizeof(c) / sizeof(c[0]), temperatureC) + a0 * exp(a1 * (temperatureC - a2) * (temperatureC - a2));
}

double convertKTypeMvToTemp(double voltageMv)
{
    if (voltageMv < -5.891) {
        voltageMv = -5.891;
    } else if (54.886 < voltageMv) {
        voltageMv = 54.886;
    }

    if (voltageMv < 0.0) {
        static const double d[] = {
            0.0000000E+00,
            2.5173462E+01,
            -1.1662878E+00,
            -1.0833638E+00,
            -8.9773540E-01,
            -3.7342377E-01,
            -8.6632643E-02,
            -1.0450598E-02,
            -5.1920577E-04
        };
        return evaluatePolynomial(d, sizeof(d) / sizeof(d[0]), voltageMv);
    }

    if (voltageMv <= 20.644) {
        static const double d[] = {
            0.0000000E+00,
            2.5083550E+01,
            7.8601060E-02,
            -2.5031310E-01,
            8.3152700E-02,
            -1.2280340E-02,
            9.8040360E-04,
            -4.4130300E-05,
            1.0577340E-06,
            -1.0527550E-08
        };
        return evaluatePolynomial(d, sizeof(d) / sizeof(d[0]), voltageMv);
    }

    static const double d[] = {
        -1.3180580E+02,
        4.8302220E+01,
        -1.6460310E+00,
        5.4647310E-02,
        -9.6507150E-04,
        8.8021930E-06,
        -3.1108100E-08
    };
    return evaluatePolynomial(d, sizeof(d) / sizeof(d[0]), voltageMv);
}

double readAverageMilliVolts(uint8_t pin)
{
    uint32_t totalMilliVolts = 0;
    for (size_t i = 0; i < kAdcSamples; ++i) {
        totalMilliVolts += analogReadMilliVolts(pin);
    }
    return static_cast<double>(totalMilliVolts) / static_cast<double>(kAdcSamples);
}
} // namespace

RoasterLoggerAnalog::RoasterLoggerAnalog(uint16_t logBufferSize, Pins pins)
    : RoasterLogger(logBufferSize)
    , pins_(pins)
{
    pinMode(pins_.environment, INPUT);
    pinMode(pins_.bean, INPUT);
    analogSetAttenuation(ADC_11db);
    analogReadResolution(kAdcResolutionBits);
}

bool RoasterLoggerAnalog::save()
{
    timeval tv;
    gettimeofday(&tv, NULL);

    const double coldJunctionMilliVoltsMeasured = readAverageMilliVolts(pins_.environment);
    const double coldJunctionVolts = coldJunctionMilliVoltsMeasured / 1000.0;
    const double coldJunctionTemperatureC = (coldJunctionVolts - kMcp9701aOffsetVolts) / kMcp9701aVoltsPerDegreeC;

    const double amplifiedMilliVoltsMeasured = readAverageMilliVolts(pins_.bean);
    const double amplifiedMilliVolts = amplifiedMilliVoltsMeasured - kThermocoupleOffsetMilliVolts;
    const double thermocoupleMilliVolts = amplifiedMilliVolts / kThermocoupleAmplifierGain;
    const double coldJunctionMilliVolts = convertKTypeTempToMv(coldJunctionTemperatureC);
    const double beanTemperatureC = convertKTypeMvToTemp(thermocoupleMilliVolts + coldJunctionMilliVolts);

    appendLog(Temperature(tv, beanTemperatureC, 0.0)); // environment temperature is not measured
    return true;
}
