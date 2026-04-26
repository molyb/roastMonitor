#pragma once

#include <WebServer.h>
#include <memory>
#include <vector>

class RoasterWebServer {

public:
    struct WebLogEntry {
        timeval time;
        double bt;
        double et;
    };

    RoasterWebServer(unsigned int port);
    bool begin(std::function<std::vector<WebLogEntry>()>);
    void handleClient(void);
    bool isInitialized() const { return isInitialized_; };

private:
    void handleLog(void);
    void handleDefault(void);

    WebServer server;
    bool isInitialized_;
    std::function<std::vector<WebLogEntry>()> callbackGetLog_;
};
