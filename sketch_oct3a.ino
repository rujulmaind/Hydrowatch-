#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MPU6050 mpu;
SoftwareSerial BT(2, 3);  // RX, TX

const int pulsePin = A0;
const int buzzerPin = 6;

int pulseValue = 0;
unsigned long lastMotionTime = 0;
int flatPulseCounter = 0;
bool alertTriggered = false;

void setup() {
  Serial.begin(9600);
  BT.begin(9600); // Bluetooth baud rate
  Wire.begin();
  mpu.initialize();
  pinMode(buzzerPin, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hydro Watch Ready!");
  display.display();
  delay(1000);
}

void loop() {
  // Read Pulse
  pulseValue = analogRead(pulsePin);
  if (pulseValue < 200) {
    flatPulseCounter++;
    if (flatPulseCounter > 20) {
      triggerAlert("No pulse");
    }
  } else {
    flatPulseCounter = 0;
    lastMotionTime = millis();
  }

  // Read MPU6050
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float totalAcc = sqrt(ax * ax + ay * ay + az * az) / 16384.0;
  if (totalAcc > 0.1) {
    lastMotionTime = millis();
  }

  if (millis() - lastMotionTime > 10000 && !alertTriggered) {
    triggerAlert("No movement");
  }

  // OLED display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Pulse: "); display.println(pulseValue);
  display.print("Accel: "); display.println(totalAcc, 2);
  display.display();

  // Send over Bluetooth
  BT.print("Pulse: "); BT.print(pulseValue);
  BT.print(" | Accel: "); BT.println(totalAcc, 2);

  delay(200);
}

void triggerAlert(String reason) {
  alertTriggered = true;
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("⚠ DANGER!");
  display.println(reason);
  display.display();

  BT.println("ALERT: " + reason);

  for (int i = 0; i < 5; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);
    delay(200);
  }
}
