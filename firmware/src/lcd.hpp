#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void displayTemperature(Adafruit_SSD1306& display, float temp);
void displayCompactTemp(Adafruit_SSD1306& display, float temp);
