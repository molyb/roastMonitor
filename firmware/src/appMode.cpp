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

std::unique_ptr<RoasterWebServer> httpServer;
std::unique_ptr<RoasterWebSocketServer> wsServer;

static bool setupNetwork(const String& ssid, const String& password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.println("Connecting to WiFi: " + ssid);
    unsigned int timeout_sec = NETWORK_SETUP_TIMEOUT_SEC; // timeout_secが0の場合は無限に待ち
    double spendTime_sec = 0.0;
    bool isConnected = false;
    bool isTimeout = timeout_sec < spendTime_sec; // 初回はfalseになるように
    while (!isTimeout && !isConnected) {
        isConnected = WiFi.status() == WL_CONNECTED;
        int interval_ms = 500;
        delay(interval_ms);
        // timeout_secが0の場合は無限に待たせるため、0より大きい場合のみspendTime_secを加算
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
    Serial.print((String)asctime(current_tm)); // asctimeの戻り値は改行文字を含む

    MDNS.begin(HOSTNAME);
    Serial.println("hostname: " HOSTNAME ".local");
    return true;
}

SetupAppResult setupApp()
{
    String ssid;
    String password;

    if (!loadConfig(ssid, password)) {
        Serial.println("Failed to load config. Please connect to AP \"RoastMonitorSetup\" and set WiFi credentials.");
        return SetupAppResult::FAILURE_LOAD_CONFIG;
    }

    httpServer = std::make_unique<RoasterWebServer>(80);
    wsServer = std::make_unique<RoasterWebSocketServer>(8080, "/ws");
    std::shared_ptr<RoasterLogger> logger = std::make_shared<RoasterLoggerAnalog>(120, 17); // 2分分のログを保存, SPI CSピン=17
    logger->startLogging(500); // 500msごとにログを保存
    if (httpServer == nullptr || wsServer == nullptr || logger == nullptr) {
        Serial.println("Failed to allocate memory.");
        return SetupAppResult::FAILURE_ALLOCATE_MEMORY;
    }

    if (setupNetwork(ssid, password)) {
        if (!httpServer->begin([logger]() -> std::vector<RoasterWebServer::WebLogEntry> {
                auto logs = logger->getLog();
                std::vector<RoasterWebServer::WebLogEntry> webLogs;
                webLogs.reserve(logs.size());
                for (const auto& log : logs) {
                    webLogs.push_back({ log.time, log.beanTemperature, log.environmentTemperature });
                }
                return webLogs;
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
        // ネットワーク設定はされていないが、モーター制御とロギングはできるようにするため、
        // HTTP/WebSocketサーバーの初期化失敗は致命的なエラーにしない
        Serial.println("Network is not configured. Skip starting HTTP/WebSocket Server.");
    }

    return SetupAppResult::SUCCESS;
}

void loopApp()
{
    if (httpServer->isInitialized()) {
        httpServer->handleClient();
    }
    if (wsServer->isInitialized()) {
        wsServer->handleClient();
    }
}
