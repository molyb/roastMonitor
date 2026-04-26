#include "configFile.h"

#include "config.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

bool loadConfig(String& ssid, String& password) {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS failed to begin.");
        return false;
    }

    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println("Config file not found. Using defaults from privateConfig.h");
        return false;
    }

    File configFile = LittleFS.open(CONFIG_FILE, "r");
    if (!configFile) {
        Serial.println("Failed to open config file.");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Serial.println("Failed to parse config file.");
        return false;
    }

    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();

    Serial.println("Config loaded from " CONFIG_FILE);
    return true;
}

bool saveConfig(const String& ssid, const String& password) {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS failed to begin.");
        return false;
    }
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["password"] = password;

    File configFile = LittleFS.open(CONFIG_FILE, "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing.");
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Failed to write to config file.");
        configFile.close();
        return false;
    }

    configFile.close();
    Serial.println("Config saved to " CONFIG_FILE);
    return true;
}
