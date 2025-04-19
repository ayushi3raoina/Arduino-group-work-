/*
 * Logic Summary:
 * - buzzerPin: turns on when distance > 50
 * - ledLow/ledMed/ledHigh: light level range indicators
 * - btConnectionLed: turns on only when btStatus == 1 and BLE is connected
 * - When not connected, simulated data is used for test/debug
 */

#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service
BLEStringCharacteristic dataChar("19B10002-E8F2-537E-4F6C-D104768A1214", BLEWrite, 32);

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// Pin setup
const int buzzerPin = 2;
const int ledLow = 3;
const int ledMed = 4;
const int ledHigh = 5;
const int btConnectionLed = 6;

// Simulated sensor values
int distance = 0;
int lightLevel = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // Add both characteristics
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(dataChar);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");

  pinMode(buzzerPin, OUTPUT);
  pinMode(ledLow, OUTPUT);
  pinMode(ledMed, OUTPUT);
  pinMode(ledHigh, OUTPUT);
  pinMode(btConnectionLed, OUTPUT);
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();
  static bool btStatus = false;

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        byte value =  switchCharacteristic.value();
        if(value ==1)   {
          btStatus = !btStatus; // Toggle status of BT
          digitalWrite(btConnectionLed, btStatus ? HIGH : LOW);
        
          Serial.println("LED on (delayed)");
          
          Serial.print("LED toggled to: ");
          Serial.println(btStatus ? "ON" : "OFF");
        }
      }

      if (dataChar.written()) {
        String received = dataChar.value();
        parseSensorData(received);
      }

      // --- Buzzer Logic ---
      digitalWrite(buzzerPin, distance > 50 ? HIGH : LOW);
      if (distance > 50) {
        tone(buzzerPin, 1000);
      } else {
        noTone(buzzerPin);
      }

      // --- Light Level LED Logic ---
      if (lightLevel < 50) {
        setLightLED(ledLow);
      } else if (lightLevel < 200) {
        setLightLED(ledMed);
      } else {
        setLightLED(ledHigh);
      }
    }

    delay(500); // Polling delay (adjust as needed)

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  } else {
    runSimulation();

    delay(500); // Polling delay (adjust as needed)
  }
}

// Utility: set only one light-level LED at a time
void setLightLED(int activePin) {
  digitalWrite(ledLow, LOW);
  digitalWrite(ledMed, LOW);
  digitalWrite(ledHigh, LOW);
  digitalWrite(activePin, HIGH);
}

void runSimulation() {
  simulateIncomingData();

  // Buzzer
  digitalWrite(buzzerPin, distance > 100 ? HIGH : LOW);

  // Light level LEDs
  if (lightLevel < 300) {
    setLightLED(ledLow);
  } else if (lightLevel < 700) {
    setLightLED(ledMed);
  } else {
    setLightLED(ledHigh);
  }
}

// Simulated incoming data (stand-in for Wi-Fi or Bluetooth)
void simulateIncomingData() {
  // Simulate values changing over time
  static int step = 0;
  step = (step + 1) % 6;

  switch (step) {
    case 0:
      distance = 90;
      lightLevel = 200;
      break;
    case 1:
      distance = 110;
      lightLevel = 500;
      break;
    case 2:
      distance = 80;
      lightLevel = 800;
      break;
    case 3:
      distance = 150;
      lightLevel = 600;
      break;
    case 4:
      distance = 40;
      lightLevel = 100;
      break;
    case 5:
      distance = 130;
      lightLevel = 750;
      break;
  }

  Serial.print("Distance: "); Serial.println(distance);
  Serial.print("Light: "); Serial.println(lightLevel);
}

void parseSensorData(String data) {
  int comma1 = data.indexOf(',');
  int comma2 = data.lastIndexOf(',');

  if (comma1 > 0 && comma2 > comma1) {
    distance = data.substring(0, comma1).toInt();
    lightLevel = data.substring(comma1 + 1, comma2).toInt();

    Serial.print("Distance: "); Serial.println(distance);
    Serial.print("Light: "); Serial.println(lightLevel);
  }
}
