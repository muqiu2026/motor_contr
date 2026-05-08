#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS    10
#define TFT_RST   9
#define TFT_DC    8
#define TFT_SDA   11
#define TFT_SCL   12
#define TFT_LED   13

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_SDA, TFT_SCL, TFT_RST);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  tft.init(240, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("ESP32-S3 Display");

  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.println("Initializing...");

  delay(1000);

  uint16_t colors[] = {
    ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE,
    ST77XX_CYAN, ST77XX_MAGENTA, ST77XX_YELLOW,
    ST77XX_WHITE, ST77XX_ORANGE
  };

  const char* colorNames[] = {
    "RED", "GREEN", "BLUE", "CYAN", "MAGENTA", "YELLOW", "WHITE", "ORANGE"
  };

  for (int i = 0; i < 8; i++) {
    tft.fillScreen(colors[i]);
    tft.setTextColor(ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setCursor(40, 100);
    tft.println(colorNames[i]);
    delay(500);
  }

  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  static int x = 0;
  static int y = 120;
  static int dx = 2;
  static int dy = 1;

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("ESP32-S3 TFT Demo");

  tft.drawCircle(x, y, 15, ST77XX_RED);
  tft.drawCircle(x + 40, y, 15, ST77XX_GREEN);
  tft.drawCircle(x + 80, y, 15, ST77XX_BLUE);

  x += dx;
  if (x > 220 || x < 15) dx = -dx;
  y += dy;
  if (y > 220 || y < 30) dy = -dy;

  delay(16);
}