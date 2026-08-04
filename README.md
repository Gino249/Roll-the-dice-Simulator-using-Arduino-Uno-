# Roll-the-dice-Simulator-using-Arduino-Uno-

## Wiring (see diagram above)

| Component | Arduino pin(s) | Notes |
|---|---|---|
| OLED SSD1306 | SDA→A4, SCL→A5, VCC→5V, GND→GND | Shared I2C bus |
| MPU6050 | SDA→A4, SCL→A5, VCC→3.3-5V, GND→GND | Same I2C bus, different address, no conflict |
| Buzzer | D3 | Same as your original code |
| 4x4 keypad | Rows→D4,D5,D6,D7 / Cols→D8,D10,D11,D12 | Needs the `Keypad` library (Library Manager → search "Keypad" by Mark Stanley) |
| L298N driver IN1/IN2 | D13 / A0 | Direction control |
| L298N driver ENA | D9 (PWM) | Speed control via `analogWrite` |
| L298N OUT1/OUT2 | → TT gear motor's two wires | Polarity only matters for spin direction |
| L298N +12V/VM | External 4–9V supply (e.g. 4×AA pack), **not** the Arduino 5V pin | The TT motor draws more current than the Uno can safely source |
| L298N GND | Shared with Arduino GND | Critical — common ground or the motor logic won't work |



**1. Enter number first**
Screen shows "Enter number 1-6 / Press # to confirm." You press a digit key (1-6), it shows on screen, then you press `#` to lock it in. If you typed something outside 1-6, it flashes "Invalid!" and makes you re-enter — you can't move on until a valid number is confirmed.

**2. Then it asks you to roll (shake) the dice**
Screen switches to "Roll the dice!" and just waits. Nothing else happens until the motion sensor detects a shake (acceleration magnitude > 1.5).

**3. Shuffling starts**
The moment a shake is detected, it runs the 10-frame shuffle animation — random dots flashing rapidly inside the dice box, with a buzzer tick on each frame — exactly like your original code's shuffle effect.

**4. Shuffle stops on a final number, then it's matched**
After the 10 shuffle frames, it picks one random result (1-6), draws the correct dot pattern for that number, plays the 3-beep result tone, and *then* compares that result against the number you entered back in step 1.

**5a. If matched → win**
Screen shows "Congratulations / You Win!" and the motor spins for 3 seconds (your reward), then the game loops back to step 1 for a new round.

**5b. If not matched → lose**
Screen shows:
```
You Lose!
HAHAHA!

Press # to restart
```
The motor does **not** run. It then just sits there — not accepting any key except `#`. Pressing any other key (letters, `*`, other digits) does nothing. Only pressing `#` sends it back to step 1 to start a brand new round.

![Diagram](Screenshot_2026-08-04-16-17-00-53_40deb401b9ffe8e1df2f1cc5ba480b12.jpg)

For Electric Shock Feature:
       [ STEP 1: ARDUINO CONTROL SIDE ]              |     [ STEP 2: HIGH-VOLTAGE STATIC SIDE ]
                                                     |
       +-----------------------+                     |             +-----------------------+

       |      ARDUINO UNO      |                     |             |    12V BATTERY PACK   |
       |                       |                     |             |    (8x AA Batteries)  |
       |  [5V]   [GND]   [D2]  |                     |             |   [ POS + ]   [ NEG - ]
       +---|-------|-------|---+                     |             +---|-----------|-------+

           |       |       |                         |                 |           |
           |       |       |  (Low-Voltage Control)  |                 |           |
           v       v       v                         |                 |           |
       +---|-------|-------|---+                     |                 |           |

       |      5V RELAY MODULE  |                     |                 |           |
       |   [VCC]  [GND]   [IN] |                     |                 |           |
       |                       |                     |                 |           |
       |   [NO]        [COM]   |                     |                 |           |
       +-----|-----------|-----+                     |                 |           |

             |           |                           |                 |           |
             |           +---------------------------|-----------------+           |
             |                                       |                             |
             +--------------------+                  |                             |

                                  |                  |                             |
                                  v                  |                             |
                       +----------|----------+       |                             |

                       |    INPUT RED wire   |       |                             |
                       |                     |       |                             |
                       |  NEGATIVE ION GEN.  |<------|-----------------------------+
                       |  (12V DC Module)    |       |                      (INPUT BLACK wire)
                       |                     |       |
                       | [OUT 1]     [OUT 2] |       |
                       +----|-----------|----+       |

                            |           |            |
                            |           |            | <=== KEEP A PHYSICAL AIR GAP OF 10cm+
   =========================|===========|============|=========================================

                            |           |            |
                            |           |            |     [ STEP 3: NOVELTY USER CONTACTS ]
                            v           v            |
                        ( Electrode A ) ( Electrode B )
                        [  Finger Pad ] [  Finger Pad ]
                        
                        *Note: Keep contacts on the SAME hand!*
## Complete Connection Guide
Here is the exact step-by-step wiring guide for your project, updated for a 5V DC Negative Ion Generator setup. This guide replaces the 12V battery block with a dedicated 5V power source for the generator while maintaining total electrical isolation.
------------------------------
## 🧱 Parts Inventory Needed

* Logic Side:
* Arduino Uno (powered by USB cable to a computer or separate power bank)
   * Standard jumper wires (Male-to-Male or Male-to-Female)
* The Switch:
* 5V Single-Channel Relay Module
* Static Side:
* 5V DC Input Negative Ion Generator Module
   * Dedicated 5V Power Supply for the generator (Either a 4x AA battery pack or a separate USB Power Bank with a hacked USB wire exposing the internal red and black power leads)
* User Interface:
* Two copper coins or aluminum foil blocks (to act as Finger Contact Pads A and B)

------------------------------
## 💻 Step 1: Wiring the Arduino Control Side (Low Voltage)
This side handles the brain logic. It uses the Arduino's 5V power supply rail to operate the electromagnetic coil inside the relay module.

   1. Power the Relay: Connect a jumper wire from the Arduino's 5V pin to the VCC (or pin marked +) on the relay module.
   2. Ground the Relay: Connect a jumper wire from any Arduino GND pin to the GND (or pin marked -) on the relay module.
   3. Signal Control Line: Connect a jumper wire from Arduino Digital Pin 2 (D2) to the IN (or SIG) pin on the relay module.

At this stage, when you run code to toggle Pin D2 HIGH and LOW, you should hear a mechanical "click" inside the relay cube, and an on-board indicator LED on the relay should flash.
------------------------------
## ⚡ Step 2: Wiring the Isolated Static Generator Side (High Voltage)
This side uses a completely independent power source. None of these wires should ever touch the Arduino board pins or the Arduino ground rails.

   1. Direct Ground Link: Take the Negative (-) wire from your dedicated 5V static power source (e.g., the black wire of your 4x AA battery pack) and connect it directly to the Input Black wire of the Negative Ion Generator.
   2. The Relay Break (Power In): Take the Positive (+) wire from your dedicated 5V static power source (e.g., the red wire of your battery pack) and route it to the relay module. Secure it into the screw terminal labeled COM (Common).
   3. The Relay Output (Power Out): Cut a fresh piece of hookup wire. Screw one end into the relay module terminal labeled NO (Normally Open). Connect the other end of this wire directly to the Input Red wire of the Negative Ion Generator.

------------------------------
## 🖐️ Step 3: Wiring the High-Voltage Target Contacts
The output wires from the generator emit the static charge. Handle these connections carefully during assembly.

   1. Electrode A: Locate the primary thick high-voltage output lead coming out of the sealed generator brick. Solder or tape this securely to Finger Contact Pad A.
   2. Electrode B: Locate the second high-voltage output lead (this is often a carbon fiber wire brush or a secondary colored wire). Solder or tape this wire securely to Finger Contact Pad B.
   3. Safety Placement: Secure both contact pads to your project housing. Ensure they are placed side-by-side so the user can easily rest their index and middle finger of the same hand on them simultaneously.

------------------------------
