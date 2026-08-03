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

// ---------------- Keypad 4x4 (lightweight manual scan, no Keypad library) ----------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {4, 5, 6, 7};  // R1-R4
byte colPins[COLS] = {8, 9, 10, 2}; // C1-C4 (moved off pin 11 to free it for the motor's ENA/PWM)

// ---------------- L298N motor driver ----------------
const int IN1 = 12;
const int IN2 = 13;
const int ENA = 11;                // must be a PWM pin
const int MOTOR_SPEED = 200;       // 0-255
const unsigned long MOTOR_RUN_TIME = 3000; // ms the motor spins on a win

void runMotor(unsigned long durationMs) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, MOTOR_SPEED);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println(F("Dispensing the"));
  display.setCursor(10, 35);
  display.println(F("reward..."));
  display.display();

  delay(durationMs);
  stopMotor();

  display.clearDisplay();
  display.display();
}

void stopMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// Scans the matrix once and returns the pressed key, or 0 if none pressed.
// Blocks briefly (~20ms) only when a key is actually held, for simple debounce.
char getKeyPress() {
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);

    for (byte c = 0; c < COLS; c++) {
      pinMode(colPins[c], INPUT_PULLUP);
      if (digitalRead(colPins[c]) == LOW) {
        delay(20); // debounce
        if (digitalRead(colPins[c]) == LOW) {
          while (digitalRead(colPins[c]) == LOW) { /* wait for release */ }
          pinMode(rowPins[r], INPUT);
          return keys[r][c];
        }
      }
      pinMode(colPins[c], INPUT);
    }

    pinMode(rowPins[r], INPUT);
  }
  return 0;
}

int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(buzzer, OUTPUT);

  Serial.print(F("Free RAM before Wire.begin(): "));
  Serial.println(freeRam());

  Wire.begin();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  stopMotor();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    Serial.print(F("Free RAM at failure: "));
    Serial.println(freeRam());
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(4, 25);
  display.println(F("Smart Dice"));
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
    char key = getKeyPress();

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
  display.println(F("Enter a number 1-6"));

  display.setTextSize(3);
  display.setCursor(56, 30);
  if (digit >= 0) {
    display.print(digit);
  } else {
    display.print(F("_"));
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
  display.println(F("Roll The"));
  display.setCursor(30, 45);
  display.println(F("Dice"));
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
  display.println(F("Rolling..."));
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
  display.println(F("Congrats"));
  display.println(F("You Win!"));
  display.display();

  runMotor(MOTOR_RUN_TIME); // spins for MOTOR_RUN_TIME ms, then stops itself

  delay(1000); // then loop() restarts the game automatically
}

void showLose() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println(F("You Lose!"));
  display.setTextSize(1);
  display.setCursor(20, 35);
  display.println(F("HAHAHA"));
  display.setCursor(0, 50);
  display.println(F("Enter # button to"));
  display.setCursor(0, 58);
  display.println(F("restart the game"));
  display.display();

  char key;
  do {
    key = getKeyPress();
  } while (key != '#');
}
