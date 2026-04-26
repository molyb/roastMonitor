#pragma once

// WebServerとESPAsyncWebServerの両方でHTTP_ANYが定義されているため、このマクロは一旦undefしておく必要がある
#ifdef HTTP_ANY
#undef HTTP_ANY
#endif
#include <ESPAsyncWebServer.h>

class RoasterWebSocketServer {

public:
    RoasterWebSocketServer(uint16_t port, const String& path);
    bool begin(std::function<void(double& tempBt, double& tmpEt)>);
    bool handleClient();
    bool isInitialized() const { return isInitialized_; };

private:
    std::function<void(double& tempBt, double& tmpEt)> readTemperature_;
    void handleWebSocketMessage(void* arg, uint8_t* data, size_t len, AsyncWebSocketClient* client);
    void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
        void* arg, uint8_t* data, size_t len);

    bool isInitialized_;
    AsyncWebServer server_;
    AsyncWebSocket ws_;
};
