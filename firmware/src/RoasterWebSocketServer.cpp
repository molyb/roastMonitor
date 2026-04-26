#include "RoasterWebSocketServer.h"

#include <ArduinoJson.h>

void RoasterWebSocketServer::handleWebSocketMessage(void* arg, uint8_t* data, size_t len, AsyncWebSocketClient* client)
{
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

        // 受信データをJSONとしてパース
        JsonDocument doc; // ArduinoJson v7
        DeserializationError error = deserializeJson(doc, data, len);

        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }

        // Artisanからのコマンド解析
        // Artisanのリクエスト形式: { "command": "getBT", "id": 12345, "machine": 0 }
        const char* command = doc["command"];
        long id = doc["id"]; // リクエストID (レスポンスにそのまま含める必要がある)

        if (command) {
            JsonDocument res;
            res["id"] = id; // リクエストと同じIDを返す
            double bt = 0, et = 0;
            readTemperature_(bt, et);
            if (strcmp(command, "getBT") == 0) {
                res["data"]["BT"] = bt;
            } else if (strcmp(command, "getET") == 0) {
                res["data"]["ET"] = et;
            } else if (strcmp(command, "getData") == 0) {
                // まとめて取得する場合のカスタム実装（Artisan設定による）
                res["data"]["BT"] = bt;
                res["data"]["ET"] = et;
            }

            // JSONを文字列化して返信
            String response;
            serializeJson(res, response);
            client->text(response);

            // デバッグ出力
            Serial.printf("CMD: %s, ID: %ld -> Resp: %s\n", command, id, response.c_str());
        }
    }
}

void RoasterWebSocketServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
    void* arg, uint8_t* data, size_t len)
{
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len, client);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

RoasterWebSocketServer::RoasterWebSocketServer(uint16_t port, const String& path)
    : server_(port)
    , ws_(path)
    , isInitialized_(false)
{
}

bool RoasterWebSocketServer::begin(std::function<void(double& tempBt, double& tmpEt)> readTemperature)
{
    readTemperature_ = readTemperature;
    server_.addHandler(&ws_);
    ws_.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
                    void* arg, uint8_t* data, size_t len) { this->onEvent(server, client, type, arg, data, len); });
    server_.begin();
    isInitialized_ = true;
    return true;
}

bool RoasterWebSocketServer::handleClient()
{
    ws_.cleanupClients();
    return true;
}
