#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- Motion sensor ----------------
MPU6050 mpu(Wire);

// ---------------- Buzzer ----------------
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
byte colPins[COLS] = {8, 10, 11, 12}; // C1-C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- Motor driver (L298N / L9110 style) ----------------
const int IN1 = 13;
const int IN2 = A0;
const int ENA = 9;          // must be a PWM pin
const int MOTOR_SPEED = 220;      // 0-255
const unsigned long MOTOR_RUN_TIME = 3000; // ms the motor spins on a win

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  stopMotor();

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

  randomSeed(analogRead(A1)); // better random seed from a floating analog pin
}

void loop() {
  int userNumber = getUserNumber();   // blocks until a valid 1-6 + # is entered
  waitForMotion();                    // shows "Roll the dice" and blocks until shaken
  int diceResult = rollDice();        // plays the shuffle animation and returns 1-6

  if (diceResult == userNumber) {
    winSequence();
  } else {
    loseSequence();                   // blocks until user presses # to restart
  }
  // loop() naturally starts a new round after this
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
      if (key >= '0' && key <= '9') {
        digitEntered = key - '0';
        showEntryScreen(digitEntered);
      } else if (key == '#') {
        if (digitEntered >= 1 && digitEntered <= 6) {
          valid = true;
        } else {
          showInvalidEntry();
          digitEntered = -1;
          showEntryScreen(digitEntered);
        }
      }
      // any other key (A,B,C,D,*) is ignored
    }
  }

  return digitEntered;
}

void showEntryScreen(int digit) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Enter number 1-6");
  display.println("Press # to confirm");

  display.setTextSize(3);
  display.setCursor(56, 30);
  if (digit >= 0) {
    display.print(digit);
  } else {
    display.print("_");
  }
  display.display();
}

void showInvalidEntry() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Invalid! Enter a");
  display.println("number from 1 to 6");
  display.display();
  tone(buzzer, 300, 400);
  delay(1200);
}

// =====================================================================
//  WAIT FOR SHAKE / MOTION
// =====================================================================
void waitForMotion() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(4, 20);
  display.println("Roll the");
  display.println(" dice!");
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

// =====================================================================
//  DICE ROLL (unchanged animation logic, now returns the result)
// =====================================================================
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
void winSequence() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 15);
  display.println("Congratulations");
  display.setCursor(4, 30);
  display.println("You Win!");
  display.display();

  runMotor(MOTOR_RUN_TIME);

  delay(2000); // let the win screen sit before a new round begins
}

void loseSequence() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 10);
  display.println("You Lose!");
  display.setCursor(4, 25);
  display.println("HAHAHA!");
  display.setCursor(4, 45);
  display.println("Press # to restart");
  display.display();

  char key;
  do {
    key = keypad.getKey();
  } while (key != '#');
}

// =====================================================================
//  MOTOR CONTROL
// =====================================================================
void runMotor(unsigned long durationMs) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, MOTOR_SPEED);
  delay(durationMs);
  stopMotor();
}

void stopMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
