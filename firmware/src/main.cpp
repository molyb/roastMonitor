#include "lcd.hpp"
// OLEDディスプレイの幅と高さ（ピクセル）
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// リセットピンの指定（I2Cの場合は通常不要なので-1を指定）
#define OLED_RESET    -1

// ディスプレイオブジェクトの宣言
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println(F("start setup"));
  // SSD1306の初期化
  // 一般的なI2Cアドレスは 0x3C ですが、動かない場合は 0x3D に変更してみてください
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306の初期化に失敗しました"));
    for(;;); // 失敗した場合はここで処理を停止
  }

  display.setTextColor(SSD1306_WHITE); // 文字色（白）
  // displayTemperature(125.5); // 例として25.5度を表示
  displayCompactTemp(display, 125.5); // 例として25.5度を表示
}

void loop() {
  // 今回は静止テキストのみなのでloop内は空です
}
