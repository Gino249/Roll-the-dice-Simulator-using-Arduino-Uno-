#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

MPU6050 mpu(Wire);

const int buzzer = 3;

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  
  Wire.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(4, 25);
  display.println("Smart Dice");
  display.display();
  delay(2000);
  
  // Initialize MPU6050
  mpu.begin();
  mpu.calcGyroOffsets(true);
}

void loop() {
  mpu.update();
  float ax = mpu.getAccX();
  float ay = mpu.getAccY();
  float az = mpu.getAccZ();
  
  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  if (magnitude > 1.5) {
    rollDice();
  }

  delay(300);
}

void rollDice() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(35, 10);
  display.println("Rolling...");
  display.display();

  for (int i = 0; i < 10; i++) {
    display.clearDisplay();
    drawShufflingDots();
    display.display();
    tone(buzzer, 1200, 100);
    delay(100);
  }

  int result = random(1, 7);
  display.clearDisplay();
  drawDiceFace(result);
  display.display();

  for (int i = 0; i < 3; i++) {
    tone(buzzer, 700, 200);
    delay(250);
  }
}

void drawShufflingDots() {
  display.drawRect(40, 15, 48, 48, WHITE);
  for (int i = 0; i < 6; i++) {
    int x = random(45, 80);
    int y = random(20, 55);
    display.fillCircle(x, y, 3, WHITE);
  }
}

void drawDiceFace(int num) {
  display.drawRect(40, 15, 48, 48, WHITE);
  int cx = 64, cy = 39; // center

  switch (num) {
    case 1:
      display.fillCircle(cx, cy, 3, WHITE);
      break;
    case 2:
      display.fillCircle(50, 25, 3, WHITE);
      display.fillCircle(78, 53, 3, WHITE);
      break;
    case 3:
      display.fillCircle(50, 25, 3, WHITE);
      display.fillCircle(cx, cy, 3, WHITE);
      display.fillCircle(78, 53, 3, WHITE);
      break;
    case 4:
      display.fillCircle(50, 25, 3, WHITE);
      display.fillCircle(78, 25, 3, WHITE);
      display.fillCircle(50, 53, 3, WHITE);
      display.fillCircle(78, 53, 3, WHITE);
      break;
    case 5:
      drawDiceFace(4);
      display.fillCircle(cx, cy, 3, WHITE);
      break;
    case 6:
      display.fillCircle(50, 25, 3, WHITE);
      display.fillCircle(78, 25, 3, WHITE);
      display.fillCircle(50, 39, 3, WHITE);
      display.fillCircle(78, 39, 3, WHITE);
      display.fillCircle(50, 53, 3, WHITE);
      display.fillCircle(78, 53, 3, WHITE);
      break;
  }
}
