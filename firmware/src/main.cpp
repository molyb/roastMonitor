#include "appMode.h"
// #include "config.h"
// #include "configMode.h"
#include <Arduino.h>

// #define MODE_CONTROL_PIN 0 // 現状MOTOR_CONTROL_PINと同じ

static enum class Mode {
    CONFIG_MODE,
    APP_MODE
} mode;

void setup()
{
    Serial.begin(115200);
    delay(1000); // シリアルモニタが安定するまで少し待つ
    Serial.println("\nstart setup()");
    SetupAppResult result = setupApp();

    // delay(2000);
    // pinMode(MODE_CONTROL_PIN, INPUT);
    // if (digitalRead(MODE_CONTROL_PIN) == HIGH) {
    //     SetupAppResult result = setupApp();
    //     // コンフィグのロードに失敗（コンフィグファイルが無い）した場合はコンフィグモードに遷移
    //     if (result == SetupAppResult::FAILURE_LOAD_CONFIG) {
    //         Serial.println("Failed to load config. Starting in config mode.");
    //         mode = Mode::CONFIG_MODE;
    //         setupConfig();
    //         return;
    //     }
    //     mode = Mode::APP_MODE;
    // } else {
    //     mode = Mode::CONFIG_MODE;
    //     setupConfig();
    // }
}

void loop()
{
    loopApp();
    // if (mode == Mode::APP_MODE) {
    //     while (true) {
    //         loopApp();
    //     }
    // } else {
    //     while (true) {
    //         loopConfig();
    //     }
    // }
}
