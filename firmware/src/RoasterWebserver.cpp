#include "RoasterWebserver.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

RoasterWebServer::RoasterWebServer(unsigned int port)
    : server(port)
    , isInitialized_(false)
{
}

bool RoasterWebServer::begin(std::function<std::vector<WebLogEntry>()> callbackGetLog)
{
    callbackGetLog_ = callbackGetLog;

    Serial.begin(115200);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS failed to begin.");
    }

    server.on("/log", [this]() -> void { this->handleLog(); });
    server.onNotFound([this]() -> void { this->handleDefault(); });

    server.begin();
    Serial.println("HTTP Server started");
    isInitialized_ = true;
    return true;
}

void RoasterWebServer::handleClient(void)
{
    if (!isInitialized_) {
        return;
    }
    server.handleClient();
}

static unsigned long long convertTimeval2millisec(const timeval& tv)
{
    // 秒部分をミリ秒に変換し、マイクロ秒部分をミリ秒に変換して加算
    return static_cast<unsigned long long>(tv.tv_sec) * 1000 + static_cast<unsigned long long>(tv.tv_usec) / 1000;
}

// {
//    "title": "roasting profile",
//    "log": {
//      [
//        {
//           "date": 1735999673568,
//           "BT": 30.1,
//           "ET": 40.0,
//        },
//        {
//           "date": 1735999674025,
//           "BT": 30.5,
//           "ET": 40.2,
//        },
//      ]
//    }
// }
void RoasterWebServer::handleLog(void)
{
    if (callbackGetLog_ == nullptr) {
        server.send(500, "text/plain", "Log retrieval callback is not set.");
        return;
    }

    std::vector<WebLogEntry> logs = callbackGetLog_();
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();

    root["title"] = "roasting profile";
    JsonArray event_writer = root["log"].to<JsonArray>();

    if (!server.hasArg("since")) {
        // sinceパラメータが無い場合は最新のデータ1つだけを返す
        if (!logs.empty()) {
            const auto& latest = logs.back();
            JsonObject event_obj = event_writer.add<JsonObject>();
            event_obj["date"] = convertTimeval2millisec(latest.time);
            event_obj["BT"] = latest.bt;
            event_obj["ET"] = latest.et;
        }
    } else {
        unsigned long long since = strtoull(server.arg("since").c_str(), NULL, 10);
        for (const auto& temp : logs) {
            unsigned long long time_ms = convertTimeval2millisec(temp.time);
            if (time_ms <= since) {
                continue;
            }

            JsonObject event_obj = event_writer.add<JsonObject>();
            if (event_obj.isNull()) {
                Serial.println("json memory is full.");
                break; // Stop adding if memory is full
            }
            event_obj["date"] = time_ms;
            event_obj["BT"] = temp.bt;
            event_obj["ET"] = temp.et;
        }
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void RoasterWebServer::handleDefault(void)
{
    // 内部ファイルシステムにファイルがあればリード
    String path = server.uri();
    if (path == String("/")) {
        path += "index.html";
    }
    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        auto convertMimeType = [](String filename) -> String {
            if (filename.endsWith(".htm"))
                return "text/html";
            else if (filename.endsWith(".html"))
                return "text/html";
            else if (filename.endsWith(".css"))
                return "text/css";
            else if (filename.endsWith(".js"))
                return "application/javascript";
            else if (filename.endsWith(".png"))
                return "image/png";
            else if (filename.endsWith(".gif"))
                return "image/gif";
            else if (filename.endsWith(".jpg"))
                return "image/jpeg";
            else if (filename.endsWith(".ico"))
                return "image/x-icon";
            else if (filename.endsWith(".xml"))
                return "text/xml";
            return "text/plain";
        };
        String contentType = convertMimeType(path);
        server.streamFile(file, contentType);
        file.close();
        return;
    }

    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    // HTTPステータスコード(404) 未検出(存在しないファイルにアクセス)
    server.send(404, "text/plain", message);
}
