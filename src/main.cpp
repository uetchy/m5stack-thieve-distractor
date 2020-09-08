// define must ahead #include <M5Stack.h>
// #define M5STACK_MPU6886
#define M5STACK_MPU9250
// #define M5STACK_MPU6050
// #define M5STACK_200Q
#include <M5Stack.h>

// Sound
#define SPEAKER_PIN 25
#define BEEP_FREQ 700  // msec
#define BEEP_DELAY 300
static bool beepEnabled = true;
static uint16_t beepDelayTimer = 0;
static uint16_t beepVolume = 10;

// Sensor
#define SAMPLE_PERIOD 20  // msec
static float motionThrethold = 0.5F;
static bool motionDetected = false;
static float accXBase = 0.0F;
static float accYBase = 0.0F;
static float accZBase = 0.0F;
static float accX = 0.0F;
static float accY = 0.0F;
static float accZ = 0.0F;

void printCenter(const char *text) {
  M5.Lcd.setCursor((M5.Lcd.width() / 2) - (M5.Lcd.textWidth(text) / 2),
                   M5.Lcd.height() / 2 - M5.Lcd.textsize * 10 / 2);
  M5.Lcd.printf(text);
}

void disableDisplay() {
  M5.Lcd.writecommand(ILI9341_DISPOFF);
  M5.Lcd.setBrightness(0);
}

void initDisplay() {
  M5.Lcd.writecommand(ILI9341_DISPON);
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);
}

void printAccel() {
  M5.Lcd.fillScreen(RED);
  M5.Lcd.setTextColor(WHITE, BLACK);
  // M5.Lcd.setCursor(20, 20);
  // M5.Lcd.printf("%5.2f   %5.2f   %5.2f", accX, accY, accZ);
  printCenter("MOTION DETECTED");
}

void beep() {
  M5.Speaker.update();

  if (beepEnabled) {
    dacWrite(SPEAKER_PIN, 0);
    delayMicroseconds(BEEP_FREQ);
    dacWrite(SPEAKER_PIN, beepVolume);
    delayMicroseconds(BEEP_FREQ);
    if (beepDelayTimer++ > BEEP_DELAY) {
      beepEnabled = false;
    }
  } else {
    delay(BEEP_DELAY);
    beepDelayTimer = 0;
    beepEnabled = true;
  }
}

void detectMotion() {
  M5.IMU.getAccelData(&accX, &accY, &accZ);

  if (abs(accXBase - accX) > motionThrethold ||
      abs(accYBase - accY) > motionThrethold ||
      abs(accZBase - accZ) > motionThrethold) {
    initDisplay();
    printAccel();
    motionDetected = true;
  }

  delay(SAMPLE_PERIOD);
}

void settings() {
  while (true) {
    M5.update();

    const int16_t height = M5.Lcd.height() - M5.Lcd.textsize * 10 - 10;
    M5.Lcd.setCursor(20, 20);
    M5.Lcd.printf("BATTERY %d%%", M5.Power.getBatteryLevel());
    M5.Lcd.setCursor(30, height);
    M5.Lcd.printf("VOL %d ", beepVolume);
    M5.Lcd.setCursor(210, height);
    M5.Lcd.printf("THD %.1f", motionThrethold);
    M5.Lcd.setCursor(147, height);
    M5.Lcd.printf("GO");

    if (M5.BtnA.wasPressed()) {
      beepVolume += 10;
      if (beepVolume > 255) {
        beepVolume = 10;
      }
    }

    if (M5.BtnC.wasPressed()) {
      motionThrethold += 0.1;
      if (motionThrethold > 1.0) {
        motionThrethold = 0.1;
      }
    }

    if (M5.BtnB.wasPressed()) {
      break;
    }
  }

  // init accel data
  printCenter("ACTIVE IN 2 SEC");
  delay(2000);
  M5.IMU.getAccelData(&accXBase, &accYBase, &accZBase);
}

void setup() {
  M5.begin(true, false, false);  // Initialize the M5Stack object
  M5.Power.begin();  // Power chip connected to gpio21, gpio22, I2C device
  M5.IMU.Init();     // Initialize IMU module

  initDisplay();
  settings();        // Show settings
  disableDisplay();  // Turn off display to increase battery life
}

void loop() {
  if (motionDetected) {
    beep();
    return;
  }

  detectMotion();
}