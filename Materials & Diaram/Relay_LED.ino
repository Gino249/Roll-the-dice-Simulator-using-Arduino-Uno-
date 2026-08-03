// Controls an LED through a relay module.
// Relay IN pin -> Arduino A0

const int RELAY_IN = A0;

// Most relay modules are ACTIVE LOW: LOW = relay ON, HIGH = relay OFF.
// If your relay behaves the opposite way, just swap RELAY_ON/RELAY_OFF below.
const int RELAY_ON  = LOW;
const int RELAY_OFF = HIGH;

void setup() {
  pinMode(RELAY_IN, OUTPUT);
  digitalWrite(RELAY_IN, RELAY_OFF); // start with LED off
}

void loop() {
  digitalWrite(RELAY_IN, RELAY_ON);  // turn LED on
  delay(2000);

  digitalWrite(RELAY_IN, RELAY_OFF); // turn LED off
  delay(2000);
}
