#include "lcd.hpp"

void displayTemperature(Adafruit_SSD1306& display, float temp) {
  display.clearDisplay();

  // 温度の数値部分（大きく表示）
  display.setTextSize(3);
  display.setCursor(0, 25);
  display.print(temp, 1);

  // 単位部分（少し小さく表示）
  display.setTextSize(2);
  display.write(247); // °
  display.setTextSize(3);
  display.print("C");

  display.display();
}

void displayCompactTemp(Adafruit_SSD1306& display, float temp) {
int integerPart = (int)temp;
  int fractionalPart = (abs((int)(temp * 10))) % 10; // 負の値にも対応できる計算

  display.clearDisplay();

  // --- 1. 整数部を描画 ---
  display.setTextSize(3);
  display.setCursor(10, 25); // 表示開始位置
  display.print(integerPart);

  // --- 2. 座標の取得 ---
  int16_t x = display.getCursorX();
  int16_t y = display.getCursorY();

  // --- 3. ドットを描画 ---
  // サイズ3の場合、文字の下端は y + 20 付近になります。
  // 1ピクセルだと小さすぎる場合は 2x2 で描画すると視認性が上がります。
  display.fillRect(x + 2, y + 18, 3, 3, SSD1306_WHITE);

  // --- 4. 小数部を描画 ---
  display.setCursor(x + 8, y); // ドットの分(約8px)右にずらす
  display.print(fractionalPart);

  // --- 5. 単位(℃)を描画 ---
  int16_t x_unit = display.getCursorX();

  // 度記号(°)を少し高い位置に小さく表示
  display.setTextSize(2);
  display.setCursor(x_unit + 5, y);
  display.write(247);

  // Cを表示
  display.setTextSize(3);
  display.setCursor(x_unit + 15, y);
  display.print("C");

  display.display();
}
