#include "appMode.h"

#include "RoasterLoggerAnalog.hpp"
#include "RoasterWebSocketServer.h"
#include "RoasterWebserver.h"
#include "config.h"
#include "configFile.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

#define JST (3600 * 9)
#define HOSTNAME "roast-monitor"
#define NETWORK_SETUP_TIMEOUT_SEC 15

namespace {
constexpr RoasterLoggerAnalog::Pins kLoggerAnalogPins{
    .environment = A1, // GPIO3_A1
    .bean = A2,        // GPIO4_A2
};
}

constexpr double kThermocoupleAmplifierGain = 100000.0 / (100.0 + 270.0);
constexpr double kThermocoupleOffsetVolts = 3.0 * 100.0 / (1000.0 + 100.0);
constexpr double kThermocoupleOffsetMilliVolts = kThermocoupleOffsetVolts * 1000.0;

std::unique_ptr<RoasterWebServer> httpServer;
std::unique_ptr<RoasterWebSocketServer> wsServer;

Ticker ticker;

static bool setupNetwork(const String& ssid, const String& password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.println("Connecting to WiFi: " + ssid);
    unsigned int timeout_sec = NETWORK_SETUP_TIMEOUT_SEC;
    double spendTime_sec = 0.0;
    bool isConnected = false;
    bool isTimeout = timeout_sec < spendTime_sec;
    while (!isTimeout && !isConnected) {
        isConnected = WiFi.status() == WL_CONNECTED;
        int interval_ms = 500;
        delay(interval_ms);
        if (0 < timeout_sec) {
            spendTime_sec += (double)interval_ms / 1000.0;
            isTimeout = timeout_sec < spendTime_sec;
        }
        Serial.print(".");
    }
    Serial.println("");
    if (isTimeout) {
        Serial.println("Timeout WiFi connection.");
        return false;
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp", "time.cloudflare.com");
    Serial.print("Sync NTP");
    time_t current_time;
    struct tm* current_tm;
    do {
        Serial.print(".");
        delay(1000);
        current_time = time(NULL);
        current_tm = localtime(&current_time);
    } while (current_tm->tm_year + 1900 < 2000);
    Serial.println("");
    Serial.print((String)asctime(current_tm));

    MDNS.begin(HOSTNAME);
    Serial.println("hostname: " HOSTNAME ".local");
    return true;
}

float convMvToTemp(float mv)
{
    // v = 0.4 + 0.0195 * temp;
    // temp = (v - 0.4)/0.0195
    return ((mv / (float)1000) - 0.4) / 0.0195;
}

float convMvToThermoCouple(float mv)
{
    return 0;
}

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

SetupAppResult setupApp()
{
    String ssid = "doremi-c344f0-2.4GHz";
    String password = "shirachanu0801";
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    // if (!loadConfig(ssid, password)) {
    //     Serial.println("Failed to load config. Please connect to AP \"RoastMonitorSetup\" and set WiFi credentials.");
    //     return SetupAppResult::FAILURE_LOAD_CONFIG;
    // }
    httpServer = std::make_unique<RoasterWebServer>(80);
    wsServer = std::make_unique<RoasterWebSocketServer>(8080, "/ws");
    std::shared_ptr<RoasterLogger> logger = std::make_shared<RoasterLoggerAnalog>(120, kLoggerAnalogPins);
    logger->startLogging(500);
    if (httpServer == nullptr || wsServer == nullptr || logger == nullptr) {
        Serial.println("Failed to allocate memory.");
        return SetupAppResult::FAILURE_ALLOCATE_MEMORY;
    }

    if (setupNetwork(ssid, password)) {
        if (!httpServer->begin(
                [logger]() -> std::vector<RoasterWebServer::WebLogEntry> {
                    auto logs = logger->getLog();
                    std::vector<RoasterWebServer::WebLogEntry> webLogs;
                    webLogs.reserve(logs.size());
                    for (const auto& log : logs) {
                        webLogs.push_back({ log.time, log.beanTemperature, log.environmentTemperature });
                    }
                    return webLogs;
                },
                [logger]() -> double {
                    return logger->getLatest().beanTemperature;
                })) {
            Serial.println("HTTP Server failed to initialize.");
            return SetupAppResult::FAILURE_START_HTTP_SERVER;
        }
        bool resultBeginWsServer = wsServer->begin([logger](double& tempBt, double& tempEt) -> void {
            RoasterLogger::Temperature temp = logger->getLatest();
            tempBt = temp.beanTemperature;
            tempEt = temp.environmentTemperature;
        });
        if (!resultBeginWsServer) {
            Serial.println("WebSocket Server failed to initialize.");
            return SetupAppResult::FAILURE_START_WS_SERVER;
        }
    } else {
        Serial.println("Network is not configured. Skip starting HTTP/WebSocket Server.");
    }

    ticker.attach_ms(1000, []() -> void {
        uint16_t value = analogRead(A0); // warm up ADC
        uint32_t mvRef = analogReadMilliVolts(A0);
        uint32_t mvTemp = analogReadMilliVolts(A1);
        uint32_t mvTC = analogReadMilliVolts(A2);

        const double amplifiedMilliVoltsMeasured = (double)mvTC;
        const double amplifiedMilliVolts = amplifiedMilliVoltsMeasured - kThermocoupleOffsetMilliVolts;
        const double thermocoupleMilliVolts = amplifiedMilliVolts / kThermocoupleAmplifierGain;
        double tempAir = convMvToTemp(mvTemp);
        Serial.println("Analog A0: " + String(value) + " (" + String(mvRef) + " mV)");
        Serial.println("Analog A1: " + String(mvTemp) + " mV -> " + String(tempAir) + "[deg]");
        Serial.println("Analog A2: " + String(mvTC) + " mV -> " + String(convertKTypeMvToTemp(thermocoupleMilliVolts)) + " [deg]");

        const double coldJunctionMilliVolts = convertKTypeTempToMv(tempAir);
        const double tc = convertKTypeMvToTemp(coldJunctionMilliVolts + thermocoupleMilliVolts);
        Serial.println("TC: " + String(tc) + " [deg]");
    });
    return SetupAppResult::SUCCESS;
}

void loopApp()
{
    if (httpServer && httpServer->isInitialized()) {
        httpServer->handleClient();
    }
    if (wsServer && wsServer->isInitialized()) {
        wsServer->handleClient();
    }
}
