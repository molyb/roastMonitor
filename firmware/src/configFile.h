#pragma once

#include <Arduino.h>

bool loadConfig(String& ssid, String& password);
bool saveConfig(const String& ssid, const String& password) ;
