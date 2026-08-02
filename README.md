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
