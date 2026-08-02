#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

MPU6050 mpu(Wire);

const int buzzer = 3;

// ---------------- Keypad 4x4 ----------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {4, 5, 6, 7};   // R1-R4
byte colPins[COLS] = {8, 9, 10, 11}; // C1-C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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
  int userNumber = getUserNumber();  // shows "Enter a number 1-6", waits for keypad entry
  waitForRoll();                     // shows "Roll The Dice", waits for motion
  int diceResult = rollDice();       // shuffling animation, returns the final face (1-6)

  if (diceResult == userNumber) {
    showWin();
  } else {
    showLose();                      // blocks until user presses # to restart
  }
}

// =====================================================================
//  NUMBER ENTRY
// =====================================================================
int getUserNumber() {
  int digitEntered = -1;
  bool valid = false;

  showEntryScreen(digitEntered);

  while (!valid) {
    char key = keypad.getKey();

    if (key) {
      if (key >= '1' && key <= '6') {
        digitEntered = key - '0';
        showEntryScreen(digitEntered);
      } else if (key == '#') {
        if (digitEntered >= 1 && digitEntered <= 6) {
          valid = true;
        }
      }
    }
  }

  return digitEntered;
}

void showEntryScreen(int digit) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Enter a number 1-6");

  display.setTextSize(3);
  display.setCursor(56, 30);
  if (digit >= 0) {
    display.print(digit);
  } else {
    display.print("_");
  }
  display.display();
}

// =====================================================================
//  WAIT FOR ROLL (motion trigger)
// =====================================================================
void waitForRoll() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 25);
  display.println("Roll The");
  display.setCursor(30, 45);
  display.println("Dice");
  display.display();

  bool moved = false;
  while (!moved) {
    mpu.update();
    float ax = mpu.getAccX();
    float ay = mpu.getAccY();
    float az = mpu.getAccZ();

    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    if (magnitude > 1.5) {
      moved = true;
    }
    delay(50);
  }
}

int rollDice() {
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

  return result;
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

// =====================================================================
//  WIN / LOSE
// =====================================================================
void showWin() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.println("Congrats");
  display.println("You Win!");
  display.display();
  delay(3000); // then loop() restarts the game automatically
}

void showLose() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("You Lose!");
  display.setTextSize(1);
  display.setCursor(20, 35);
  display.println("HAHAHA");
  display.setCursor(0, 50);
  display.println("Enter # button to");
  display.setCursor(0, 58);
  display.println("restart the game");
  display.display();

  char key;
  do {
    key = keypad.getKey();
  } while (key != '#');
}
