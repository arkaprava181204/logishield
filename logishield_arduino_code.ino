#define OLEDDISPLAY

#include <Wire.h>
#include <SSD1306.h>
#include <qrcodeoled.h>
#include <RTClib.h>

#include "LightProximityAndGesture.h"
#include "BarometricPressure.h"

// =====================================================
// I2C
// =====================================================

#define MPU_ADDR 0x69
#define OLED_ADDR 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// =====================================================
// BUTTON
// =====================================================

#define SWITCH_PIN 4

// =====================================================
// OLED
// =====================================================

SSD1306 display(
  OLED_ADDR,
  SDA_PIN,
  SCL_PIN);

QRcodeOled qrcode(&display);

// =====================================================
// DS3231 RTC
// =====================================================

RTC_DS3231 rtc;

bool rtcOK = false;

// Valid RTC range used by this project.
// This prevents corrupted RTC values such as
// 2161-43-10 or 2000-00-00 from being accepted.

const int RTC_MIN_YEAR = 2020;
const int RTC_MAX_YEAR = 2099;

// =====================================================
// SENSORS
// =====================================================

LightProximityAndGesture apds;

BarometricPressure bmp180(HIGH_RESOLUTION);

// =====================================================
// TIMING
// =====================================================

const unsigned long DOUBLE_PRESS_WINDOW = 1200;
const unsigned long TRIPLE_PRESS_WINDOW = 1200;

const unsigned long COUNTDOWN_TIME = 40000;

const unsigned long BUTTON_DEBOUNCE = 60;

// =====================================================
// SENSOR SAMPLING
// =====================================================

const unsigned long SENSOR_INTERVAL = 250;

unsigned long lastSensorRead = 0;

// =====================================================
// TAMPER SENSITIVITY
// =====================================================

const int REQUIRED_ABNORMAL_GROUPS = 2;

const int REQUIRED_TAMPER_READINGS = 3;

const int REQUIRED_NORMAL_READINGS = 5;

// =====================================================
// ACCELERATION THRESHOLDS
// =====================================================

const float ACCEL_THRESHOLD = 0.35;

// =====================================================
// GYROSCOPE THRESHOLDS
// =====================================================

const float GYRO_THRESHOLD = 20.0;

// =====================================================
// RGB THRESHOLDS
// =====================================================

const float RGB_THRESHOLD = 20.0;

const float RGB_ABSOLUTE_THRESHOLD = 10.0;

// =====================================================
// PRESSURE
// =====================================================

const float PRESSURE_PERCENT_THRESHOLD = 8.0;

const float PRESSURE_ABSOLUTE_THRESHOLD = 0.015;

// =====================================================
// MPU6050 SCALE
// =====================================================

const float ACCEL_SCALE = 16384.0;
const float GYRO_SCALE = 131.0;

// =====================================================
// RAW MPU VALUES
// =====================================================

int16_t rawAx;
int16_t rawAy;
int16_t rawAz;

int16_t rawGx;
int16_t rawGy;
int16_t rawGz;

// =====================================================
// FILTERED MPU VALUES
// =====================================================

float ax_g;
float ay_g;
float az_g;

float gx_dps;
float gy_dps;
float gz_dps;

// =====================================================
// REFERENCE VALUES
// =====================================================

float ref_ax_g;
float ref_ay_g;
float ref_az_g;

float ref_gx_dps;
float ref_gy_dps;
float ref_gz_dps;

uint16_t redValue;
uint16_t greenValue;
uint16_t blueValue;

uint16_t ref_redValue;
uint16_t ref_greenValue;
uint16_t ref_blueValue;

float bmpPressure;
float ref_bmpPressure;

// =====================================================
// TAMPER DIFFERENCES
// =====================================================

float tamper_d_ax;
float tamper_d_ay;
float tamper_d_az;

float tamper_d_gx;
float tamper_d_gy;
float tamper_d_gz;

float tamper_d_red;
float tamper_d_green;
float tamper_d_blue;

float tamper_d_pressure;

// =====================================================
// TIMESTAMP
// =====================================================

String tamperDate = "";
String tamperTime = "";

// =====================================================
// MOVEMENT
// =====================================================

String movementID = "S001";

// =====================================================
// TAMPER HISTORY
// =====================================================

String tamperHistory = "[]";

unsigned long tamperEventCount = 0;

// =====================================================
// TAMPER STATE
// =====================================================

bool tamperArmed = true;
bool tamperDetected = false;

int tamperViolationCount = 0;
int tamperNormalCount = 0;

// =====================================================
// SYSTEM STATE
// =====================================================

bool countdownRunning = false;
bool monitoring = false;
bool referenceCaptured = false;

unsigned long countdownStart = 0;

// =====================================================
// BUTTON STATE
// =====================================================

bool buttonStableState = HIGH;
bool buttonLastReading = HIGH;

unsigned long buttonLastChange = 0;

int pressCount = 0;
unsigned long firstPressTime = 0;

// =====================================================
// QR STATE
// =====================================================

bool qrShowing = false;

// =====================================================
// KALMAN FILTER
// =====================================================

class KalmanFilter {
public:

  float Q;
  float R;

  float X;
  float P;
  float K;

  KalmanFilter(
    float processNoise,
    float measurementNoise) {
    Q = processNoise;
    R = measurementNoise;

    X = 0.0;
    P = 1.0;
    K = 0.0;
  }

  float update(float measurement) {
    P = P + Q;

    K = P / (P + R);

    X = X + K * (measurement - X);

    P = (1.0 - K) * P;

    return X;
  }
};

// =====================================================
// KALMAN OBJECTS
// =====================================================

KalmanFilter kalmanAx(0.005, 0.05);
KalmanFilter kalmanAy(0.005, 0.05);
KalmanFilter kalmanAz(0.005, 0.05);

KalmanFilter kalmanGx(0.01, 0.1);
KalmanFilter kalmanGy(0.01, 0.1);
KalmanFilter kalmanGz(0.01, 0.1);

// =====================================================
// RTC - LEAP YEAR
// =====================================================

bool isLeapYear(int year) {
  return (
    (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

// =====================================================
// RTC - DAYS IN MONTH
// =====================================================

int daysInMonth(
  int year,
  int month) {
  switch (month) {
    case 1:
      return 31;

    case 2:
      return isLeapYear(year) ? 29 : 28;

    case 3:
      return 31;

    case 4:
      return 30;

    case 5:
      return 31;

    case 6:
      return 30;

    case 7:
      return 31;

    case 8:
      return 31;

    case 9:
      return 30;

    case 10:
      return 31;

    case 11:
      return 30;

    case 12:
      return 31;
  }

  return 0;
}

// =====================================================
// RTC - VALIDATE DATE/TIME
// =====================================================

bool isValidRTCDateTime(
  const DateTime &dt) {
  int year = dt.year();
  int month = dt.month();
  int day = dt.day();

  int hour = dt.hour();
  int minute = dt.minute();
  int second = dt.second();

  // ---------------------------------------------------
  // YEAR
  // ---------------------------------------------------

  if (
    year < RTC_MIN_YEAR || year > RTC_MAX_YEAR) {
    return false;
  }

  // ---------------------------------------------------
  // MONTH
  // ---------------------------------------------------

  if (
    month < 1 || month > 12) {
    return false;
  }

  // ---------------------------------------------------
  // DAY
  // ---------------------------------------------------

  int maximumDay = daysInMonth(
    year,
    month);

  if (
    day < 1 || day > maximumDay) {
    return false;
  }

  // ---------------------------------------------------
  // TIME
  // ---------------------------------------------------

  if (
    hour < 0 || hour > 23) {
    return false;
  }

  if (
    minute < 0 || minute > 59) {
    return false;
  }

  if (
    second < 0 || second > 59) {
    return false;
  }

  return true;
}

// =====================================================
// RTC - PRINT DATE/TIME
// =====================================================

void printRTC(
  const DateTime &now) {
  Serial.print("RTC DATE: ");

  Serial.print(now.year());

  Serial.print("-");

  if (now.month() < 10)
    Serial.print("0");

  Serial.print(now.month());

  Serial.print("-");

  if (now.day() < 10)
    Serial.print("0");

  Serial.println(now.day());

  Serial.print("RTC TIME: ");

  if (now.hour() < 10)
    Serial.print("0");

  Serial.print(now.hour());

  Serial.print(":");

  if (now.minute() < 10)
    Serial.print("0");

  Serial.print(now.minute());

  Serial.print(":");

  if (now.second() < 10)
    Serial.print("0");

  Serial.println(now.second());
}

// =====================================================
// RTC - SET COMPILE TIME
// =====================================================

void setRTCToCompileTime() {
  Serial.println();
  Serial.println(
    "SETTING DS3231 TO COMPILE DATE/TIME...");

  DateTime compileTime(
    F(__DATE__),
    F(__TIME__));

  Serial.print("COMPILE DATE/TIME: ");

  printRTC(compileTime);

  rtc.adjust(
    compileTime);

  delay(100);

  DateTime verifyTime = rtc.now();

  Serial.println();
  Serial.println(
    "RTC AFTER ADJUST:");

  printRTC(verifyTime);

  if (
    isValidRTCDateTime(
      verifyTime)) {
    Serial.println(
      "RTC DATE/TIME VALID");

    rtcOK = true;
  } else {
    Serial.println(
      "RTC DATE/TIME STILL INVALID!");

    rtcOK = false;
  }
}

// =====================================================
// RTC INITIALIZATION
// =====================================================

bool initializeRTC() {
  Serial.println();
  Serial.println(
    "==============================");

  Serial.println(
    "INITIALIZING DS3231");

  Serial.println(
    "==============================");

  // ---------------------------------------------------
  // START RTC
  // ---------------------------------------------------

  if (
    !rtc.begin()) {
    Serial.println(
      "DS3231 ERROR: RTC NOT FOUND");

    rtcOK = false;

    return false;
  }

  Serial.println(
    "DS3231 FOUND");

  // ---------------------------------------------------
  // READ CURRENT RTC VALUE
  // ---------------------------------------------------

  DateTime now = rtc.now();

  Serial.println(
    "RAW RTC VALUE:");

  printRTC(now);

  // ---------------------------------------------------
  // CHECK LOST POWER
  // ---------------------------------------------------

  bool powerLost = rtc.lostPower();

  if (powerLost) {
    Serial.println(
      "DS3231 LOST POWER");
  } else {
    Serial.println(
      "DS3231 BACKUP POWER OK");
  }

  // ---------------------------------------------------
  // VALIDATE DATE/TIME
  // ---------------------------------------------------

  bool dateValid =
    isValidRTCDateTime(now);

  if (!dateValid) {
    Serial.println();
    Serial.println(
      "********************************");

    Serial.println(
      "INVALID RTC DATE/TIME DETECTED");

    Serial.println(
      "********************************");

    Serial.println(
      "RTC VALUE WILL BE CORRECTED.");
  }

  // ---------------------------------------------------
  // CORRECT RTC IF NECESSARY
  // ---------------------------------------------------

  if (
    powerLost || !dateValid) {
    setRTCToCompileTime();
  } else {
    rtcOK = true;

    Serial.println(
      "DS3231 DATE/TIME VALID");
  }

  // ---------------------------------------------------
  // FINAL RTC CHECK
  // ---------------------------------------------------

  DateTime finalTime = rtc.now();

  Serial.println();
  Serial.println(
    "==============================");

  Serial.println(
    "FINAL RTC VALUE");

  Serial.println(
    "==============================");

  printRTC(finalTime);

  if (
    !isValidRTCDateTime(
      finalTime)) {
    Serial.println(
      "ERROR: RTC STILL INVALID!");

    rtcOK = false;

    return false;
  }

  rtcOK = true;

  Serial.println(
    "DS3231 RTC READY");

  return true;
}

// =====================================================
// GET RTC TIMESTAMP
// =====================================================

bool getTimestamp() {
  if (!rtcOK) {
    Serial.println(
      "RTC ERROR: RTC NOT READY");

    tamperDate = "RTC_ERROR";
    tamperTime = "RTC_ERROR";

    return false;
  }

  DateTime now = rtc.now();

  // ---------------------------------------------------
  // VALIDATE CURRENT RTC
  // ---------------------------------------------------

  if (
    !isValidRTCDateTime(now)) {
    Serial.println();
    Serial.println(
      "********************************");

    Serial.println(
      "RTC BECAME INVALID!");

    Serial.println(
      "********************************");

    printRTC(now);

    Serial.println(
      "RESETTING RTC...");

    setRTCToCompileTime();

    if (!rtcOK) {
      tamperDate = "RTC_ERROR";
      tamperTime = "RTC_ERROR";

      return false;
    }

    now = rtc.now();

    if (
      !isValidRTCDateTime(now)) {
      tamperDate = "RTC_ERROR";
      tamperTime = "RTC_ERROR";

      return false;
    }
  }

  // ---------------------------------------------------
  // DATE
  // ---------------------------------------------------

  char dateBuffer[11];

  snprintf(
    dateBuffer,
    sizeof(dateBuffer),
    "%04d-%02d-%02d",
    now.year(),
    now.month(),
    now.day());

  tamperDate = String(
    dateBuffer);

  // ---------------------------------------------------
  // TIME
  // ---------------------------------------------------

  char timeBuffer[9];

  snprintf(
    timeBuffer,
    sizeof(timeBuffer),
    "%02d:%02d:%02d",
    now.hour(),
    now.minute(),
    now.second());

  tamperTime = String(
    timeBuffer);

  return true;
}

// =====================================================
// DISPLAY SETUP
// =====================================================

void setupDisplay() {
  display.init();

  display.clear();

  display.flipScreenVertically();

  display.display();
}

// =====================================================
// DISPLAY CENTERED TEXT
// =====================================================

void displayCenteredText(
  String text,
  unsigned long duration) {
  display.clear();

  display.setFont(
    ArialMT_Plain_16);

  int width =
    display.getStringWidth(
      text);

  int x =
    (display.getWidth() - width) / 2;

  int y = 24;

  display.drawString(
    x,
    y,
    text);

  display.display();

  delay(duration);

  display.clear();

  display.display();
}

// =====================================================
// READ MPU6050
// =====================================================

bool readMPU() {
  Wire.beginTransmission(
    MPU_ADDR);

  Wire.write(
    0x3B);

  if (
    Wire.endTransmission(false) != 0) {
    return false;
  }

  uint8_t bytes =
    Wire.requestFrom(
      MPU_ADDR,
      14);

  if (bytes < 14) {
    return false;
  }

  rawAx =
    ((int16_t)Wire.read() << 8) | Wire.read();

  rawAy =
    ((int16_t)Wire.read() << 8) | Wire.read();

  rawAz =
    ((int16_t)Wire.read() << 8) | Wire.read();

  // Skip temperature

  Wire.read();
  Wire.read();

  rawGx =
    ((int16_t)Wire.read() << 8) | Wire.read();

  rawGy =
    ((int16_t)Wire.read() << 8) | Wire.read();

  rawGz =
    ((int16_t)Wire.read() << 8) | Wire.read();

  // ---------------------------------------------------
  // CONVERT
  // ---------------------------------------------------

  float ax =
    rawAx / ACCEL_SCALE;

  float ay =
    rawAy / ACCEL_SCALE;

  float az =
    rawAz / ACCEL_SCALE;

  float gx =
    rawGx / GYRO_SCALE;

  float gy =
    rawGy / GYRO_SCALE;

  float gz =
    rawGz / GYRO_SCALE;

  // ---------------------------------------------------
  // FILTER
  // ---------------------------------------------------

  ax_g =
    kalmanAx.update(ax);

  ay_g =
    kalmanAy.update(ay);

  az_g =
    kalmanAz.update(az);

  gx_dps =
    kalmanGx.update(gx);

  gy_dps =
    kalmanGy.update(gy);

  gz_dps =
    kalmanGz.update(gz);

  return true;
}

// =====================================================
// READ RGB
// =====================================================

void readRGB() {
  // Use the library's combined RGB reader.
  // The supplied LightProximityAndGesture library reads
  // Red, Green and Blue through its APDS9960 routines.
  uint16_t *rgb = apds.getRGBProportion(false);

  if (rgb != nullptr) {
    redValue   = rgb[0];
    greenValue = rgb[1];
    blueValue  = rgb[2];
  } else {
    redValue   = 0;
    greenValue = 0;
    blueValue  = 0;
  }
}

// =====================================================
// READ PRESSURE
// =====================================================

void readBMP180() {
  bmpPressure =
    bmp180.getPressureBar(false);
}

// =====================================================
// READ ALL SENSORS
// =====================================================

void readAllSensors() {
  readMPU();

  readRGB();

  readBMP180();
}

// =====================================================
// AVERAGE REFERENCE
// =====================================================

void captureReference() {
  const int samples = 40;

  float sumAx = 0;
  float sumAy = 0;
  float sumAz = 0;

  float sumGx = 0;
  float sumGy = 0;
  float sumGz = 0;

  float sumRed = 0;
  float sumGreen = 0;
  float sumBlue = 0;

  float sumPressure = 0;

  Serial.println();

  Serial.println(
    "CAPTURING STABLE REFERENCE...");

  for (
    int i = 0;
    i < samples;
    i++) {
    readAllSensors();

    sumAx += ax_g;
    sumAy += ay_g;
    sumAz += az_g;

    sumGx += gx_dps;
    sumGy += gy_dps;
    sumGz += gz_dps;

    sumRed += redValue;
    sumGreen += greenValue;
    sumBlue += blueValue;

    sumPressure += bmpPressure;

    delay(30);
  }

  ref_ax_g =
    sumAx / samples;

  ref_ay_g =
    sumAy / samples;

  ref_az_g =
    sumAz / samples;

  ref_gx_dps =
    sumGx / samples;

  ref_gy_dps =
    sumGy / samples;

  ref_gz_dps =
    sumGz / samples;

  ref_redValue =
    sumRed / samples;

  ref_greenValue =
    sumGreen / samples;

  ref_blueValue =
    sumBlue / samples;

  ref_bmpPressure =
    sumPressure / samples;

  referenceCaptured = true;

  Serial.println();
  Serial.println(
    "==============================");

  Serial.println(
    "REFERENCE CAPTURED");

  Serial.println(
    "==============================");

  Serial.print("ACC: ");

  Serial.print(
    ref_ax_g,
    3);

  Serial.print(" , ");

  Serial.print(
    ref_ay_g,
    3);

  Serial.print(" , ");

  Serial.println(
    ref_az_g,
    3);

  Serial.print("GYRO: ");

  Serial.print(
    ref_gx_dps,
    2);

  Serial.print(" , ");

  Serial.print(
    ref_gy_dps,
    2);

  Serial.print(" , ");

  Serial.println(
    ref_gz_dps,
    2);

  Serial.print("RGB: ");

  Serial.print(
    ref_redValue);

  Serial.print(" ");

  Serial.print(
    ref_greenValue);

  Serial.print(" ");

  Serial.println(
    ref_blueValue);

  Serial.print("PRESSURE: ");

  Serial.println(
    ref_bmpPressure,
    4);
}

// =====================================================
// ACCELERATION CHANGE
// =====================================================

float accelerationChange(
  float current,
  float reference) {
  return fabs(
    current - reference);
}

// =====================================================
// GYRO CHANGE
// =====================================================

float gyroChange(
  float current,
  float reference) {
  return fabs(
    current - reference);
}

// =====================================================
// RGB PERCENTAGE
// =====================================================

float rgbPercentage(
  float current,
  float reference) {
  float absoluteDifference =
    fabs(
      current - reference);

  if (
    reference < 5) {
    return absoluteDifference;
  }

  return (
           absoluteDifference / reference)
         * 100.0;
}

// =====================================================
// PRESSURE CHANGE
// =====================================================

float pressurePercentage() {
  if (
    fabs(ref_bmpPressure) < 0.001) {
    return 0;
  }

  return (
           fabs(
             bmpPressure - ref_bmpPressure)
           / fabs(ref_bmpPressure))
         * 100.0;
}

// =====================================================
// APPEND TAMPER EVENT
// =====================================================

void appendTamperEvent() {
  // ---------------------------------------------------
  // GET VALID RTC TIMESTAMP
  // ---------------------------------------------------

  if (!getTimestamp()) {
    Serial.println(
      "WARNING: TAMPER EVENT HAS NO VALID RTC TIME");
  }

  String eventJSON;

  eventJSON.reserve(400);

  eventJSON = "{";

  // ---------------------------------------------------
  // DATE
  // ---------------------------------------------------

  eventJSON +=
    "\"DATE\":\"" + tamperDate + "\"";

  // ---------------------------------------------------
  // TIME
  // ---------------------------------------------------

  eventJSON +=
    ",\"TIME\":\"" + tamperTime + "\"";

  // ---------------------------------------------------
  // PRESSURE
  // ---------------------------------------------------

  eventJSON +=
    ",\"P\":" + String(tamper_d_pressure, 1);

  // ---------------------------------------------------
  // MOVEMENT
  // ---------------------------------------------------

  eventJSON +=
    ",\"M\":\"" + movementID + "\"";

  // ---------------------------------------------------
  // ACCELERATION
  // ---------------------------------------------------

  eventJSON +=
    ",\"AX\":" + String(tamper_d_ax, 2);

  eventJSON +=
    ",\"AY\":" + String(tamper_d_ay, 2);

  eventJSON +=
    ",\"AZ\":" + String(tamper_d_az, 2);

  // ---------------------------------------------------
  // GYROSCOPE
  // ---------------------------------------------------

  eventJSON +=
    ",\"GX\":" + String(tamper_d_gx, 1);

  eventJSON +=
    ",\"GY\":" + String(tamper_d_gy, 1);

  eventJSON +=
    ",\"GZ\":" + String(tamper_d_gz, 1);

  // ---------------------------------------------------
  // RGB
  // ---------------------------------------------------

  eventJSON +=
    ",\"R\":" + String(tamper_d_red, 0);

  eventJSON +=
    ",\"G\":" + String(tamper_d_green, 0);

  eventJSON +=
    ",\"B\":" + String(tamper_d_blue, 0);

  eventJSON += "}";

  // ---------------------------------------------------
  // STORE EVENT
  // ---------------------------------------------------

  if (
    tamperEventCount == 0) {
    tamperHistory =
      "[" + eventJSON + "]";
  } else {
    if (
      tamperHistory.endsWith("]")) {
      tamperHistory.remove(
        tamperHistory.length() - 1);
    }

    tamperHistory += ",";

    tamperHistory +=
      eventJSON;

    tamperHistory += "]";
  }

  tamperEventCount++;

  // ---------------------------------------------------
  // SERIAL OUTPUT
  // ---------------------------------------------------

  Serial.println();

  Serial.println(
    "******************************");

  Serial.print(
    "TAMPER EVENT #");

  Serial.println(
    tamperEventCount);

  Serial.print(
    "DATE: ");

  Serial.println(
    tamperDate);

  Serial.print(
    "TIME: ");

  Serial.println(
    tamperTime);

  Serial.println(
    eventJSON);

  Serial.println(
    "******************************");
}

// =====================================================
// CHECK TAMPER
// =====================================================

bool checkTamper() {
  // ---------------------------------------------------
  // ACCELERATION
  // ---------------------------------------------------

  float dAx =
    accelerationChange(
      ax_g,
      ref_ax_g);

  float dAy =
    accelerationChange(
      ay_g,
      ref_ay_g);

  float dAz =
    accelerationChange(
      az_g,
      ref_az_g);

  bool accelerationAbnormal =
    (dAx > ACCEL_THRESHOLD || dAy > ACCEL_THRESHOLD || dAz > ACCEL_THRESHOLD);

  // ---------------------------------------------------
  // GYROSCOPE
  // ---------------------------------------------------

  float dGx =
    gyroChange(
      gx_dps,
      ref_gx_dps);

  float dGy =
    gyroChange(
      gy_dps,
      ref_gy_dps);

  float dGz =
    gyroChange(
      gz_dps,
      ref_gz_dps);

  bool gyroAbnormal =
    (dGx > GYRO_THRESHOLD || dGy > GYRO_THRESHOLD || dGz > GYRO_THRESHOLD);

  // ---------------------------------------------------
  // RGB
  // ---------------------------------------------------

  float dRed =
    rgbPercentage(
      redValue,
      ref_redValue);

  float dGreen =
    rgbPercentage(
      greenValue,
      ref_greenValue);

  float dBlue =
    rgbPercentage(
      blueValue,
      ref_blueValue);

  bool rgbAbnormal =
    (dRed > RGB_THRESHOLD || dGreen > RGB_THRESHOLD || dBlue > RGB_THRESHOLD)
    && (fabs(
          (float)redValue - (float)ref_redValue)
          > RGB_ABSOLUTE_THRESHOLD

        ||

        fabs(
          (float)greenValue - (float)ref_greenValue)
          > RGB_ABSOLUTE_THRESHOLD

        ||

        fabs(
          (float)blueValue - (float)ref_blueValue)
          > RGB_ABSOLUTE_THRESHOLD);

  // ---------------------------------------------------
  // PRESSURE
  // ---------------------------------------------------

  float dPressure =
    pressurePercentage();

  bool pressureAbnormal =
    (dPressure > PRESSURE_PERCENT_THRESHOLD)
    && (fabs(
          bmpPressure - ref_bmpPressure)
        > PRESSURE_ABSOLUTE_THRESHOLD);

  // ---------------------------------------------------
  // COUNT ABNORMAL GROUPS
  // ---------------------------------------------------

  int abnormalGroups = 0;

  if (accelerationAbnormal)
    abnormalGroups++;

  if (gyroAbnormal)
    abnormalGroups++;

  if (rgbAbnormal)
    abnormalGroups++;

  if (pressureAbnormal)
    abnormalGroups++;

  bool detected =
    rgbAbnormal ||
    (abnormalGroups >= REQUIRED_ABNORMAL_GROUPS);

  // ---------------------------------------------------
  // SERIAL DEBUG
  // ---------------------------------------------------

  Serial.print(
    "Groups abnormal: ");

  Serial.println(
    abnormalGroups);

  Serial.print(
    "ACC change: ");

  Serial.print(
    dAx,
    3);

  Serial.println(
    " g");

  Serial.print(
    "GYRO change: ");

  Serial.print(
    dGx,
    1);

  Serial.println(
    " deg/s");

  Serial.print(
    "RGB: ");

  Serial.print(
    dRed,
    1);

  Serial.print("% ");

  Serial.print(
    dGreen,
    1);

  Serial.print("% ");

  Serial.print(
    dBlue,
    1);

  Serial.println("%");

  Serial.print(
    "PRESSURE: ");

  Serial.print(
    dPressure,
    1);

  Serial.println("%");

  // ---------------------------------------------------
  // ABNORMAL
  // ---------------------------------------------------

  if (detected) {
    tamperViolationCount++;

    tamperNormalCount = 0;

    Serial.print(
      "TAMPER WARNING ");

    Serial.print(
      tamperViolationCount);

    Serial.print("/");

    Serial.println(
      REQUIRED_TAMPER_READINGS);

    // -------------------------------------------------
    // CONFIRM EVENT
    // -------------------------------------------------

    if (
      tamperArmed && tamperViolationCount >= REQUIRED_TAMPER_READINGS) {
      // -----------------------------------------------
      // SAVE SENSOR VALUES
      // -----------------------------------------------

      tamper_d_ax = dAx;
      tamper_d_ay = dAy;
      tamper_d_az = dAz;

      tamper_d_gx = dGx;
      tamper_d_gy = dGy;
      tamper_d_gz = dGz;

      tamper_d_red = dRed;
      tamper_d_green = dGreen;
      tamper_d_blue = dBlue;

      tamper_d_pressure =
        dPressure;

      // -----------------------------------------------
      // STORE EVENT
      // -----------------------------------------------

      appendTamperEvent();

      // -----------------------------------------------
      // MARK DETECTED
      // -----------------------------------------------

      tamperDetected = true;

      // -----------------------------------------------
      // DISARM
      // -----------------------------------------------

      tamperArmed = false;

      tamperViolationCount = 0;

      tamperNormalCount = 0;

      return true;
    }
  }

  // ---------------------------------------------------
  // NORMAL
  // ---------------------------------------------------

  else {
    tamperViolationCount = 0;

    if (!tamperArmed) {
      tamperNormalCount++;

      Serial.print(
        "NORMAL ");

      Serial.print(
        tamperNormalCount);

      Serial.print("/");

      Serial.println(
        REQUIRED_NORMAL_READINGS);

      if (
        tamperNormalCount >= REQUIRED_NORMAL_READINGS) {
        tamperArmed = true;

        tamperDetected = false;

        tamperNormalCount = 0;

        Serial.println(
          "TAMPER DETECTOR RE-ARMED");
      }
    }
  }

  return false;
}

// =====================================================
// NORMAL DISPLAY
// =====================================================

void displayNormal() {
  display.clear();

  display.setFont(
    ArialMT_Plain_10);

  display.drawString(
    0,
    0,
    "A:" + String(ax_g, 2) + "," + String(ay_g, 2) + "," + String(az_g, 2));

  display.drawString(
    0,
    12,
    "G:" + String(gx_dps, 1) + "," + String(gy_dps, 1) + "," + String(gz_dps, 1));

  display.drawString(
    0,
    24,
    "R:" + String(redValue) + " G:" + String(greenValue) + " B:" + String(blueValue));

  display.drawString(
    0,
    36,
    "P:" + String(bmpPressure, 3) + " bar");

  if (
    tamperEventCount == 0) {
    display.drawString(
      0,
      52,
      "STATUS: NORMAL");
  } else {
    display.drawString(
      0,
      52,
      "TAMPERS: " + String(tamperEventCount));
  }

  display.display();
}

// =====================================================
// TAMPER DISPLAY
// =====================================================

void displayTamperEvent() {
  display.clear();

  display.setFont(
    ArialMT_Plain_16);

  display.drawString(
    18,
    0,
    "TAMPER!");

  display.setFont(
    ArialMT_Plain_10);

  display.drawString(
    0,
    23,
    "Event #" + String(tamperEventCount));

  display.drawString(
    0,
    37,
    "Monitoring continues");

  display.drawString(
    0,
    50,
    "3 clicks = QR");

  display.display();
}

// =====================================================
// COUNTDOWN DISPLAY
// =====================================================

void countdownDisplay() {
  unsigned long elapsed =
    millis() - countdownStart;

  // ---------------------------------------------------
  // COMPLETE
  // ---------------------------------------------------

  if (
    elapsed >= COUNTDOWN_TIME) {
    countdownRunning = false;

    display.clear();

    display.setFont(
      ArialMT_Plain_10);

    display.drawString(
      0,
      20,
      "Capturing reference...");

    display.display();

    delay(500);

    captureReference();

    display.clear();

    display.drawString(
      0,
      10,
      "REFERENCE READY");

    display.drawString(
      0,
      27,
      "Monitoring started");

    display.drawString(
      0,
      44,
      "Low sensitivity mode");

    display.display();

    delay(1500);

    monitoring = true;

    tamperArmed = true;
    tamperDetected = false;

    tamperViolationCount = 0;
    tamperNormalCount = 0;

    return;
  }

  // ---------------------------------------------------
  // REMAINING
  // ---------------------------------------------------

  unsigned long remaining =
    (COUNTDOWN_TIME - elapsed + 999) / 1000;

  display.clear();

  display.setFont(
    ArialMT_Plain_10);

  display.drawString(
    0,
    0,
    "CALIBRATION");

  display.drawString(
    0,
    13,
    "Keep device still");

  display.setFont(
    ArialMT_Plain_24);

  display.drawString(
    45,
    32,
    String(remaining) + "s");

  display.display();
}

// =====================================================
// SHOW QR
// =====================================================

void showQR() {
  if (
    tamperEventCount == 0) {
    displayCenteredText(
      "No Tamper Data",
      1500);

    return;
  }

  String json =
    tamperHistory;

  Serial.println();

  Serial.println(
    "==============================");

  Serial.println(
    "QR CODE REQUESTED");

  Serial.print(
    "Events: ");

  Serial.println(
    tamperEventCount);

  Serial.print(
    "Data length: ");

  Serial.println(
    json.length());

  Serial.println(
    json);

  Serial.println(
    "==============================");

  display.clear();

  display.display();

  qrcode.init();

  qrcode.create(
    json);

  qrShowing = true;

  Serial.println(
    "QR DISPLAYED");

  Serial.println(
    "Press button to return.");
}

// =====================================================
// RESET TAMPER HISTORY
// =====================================================

void resetTamperHistory() {
  tamperHistory = "[]";

  tamperEventCount = 0;

  tamperArmed = true;

  tamperDetected = false;

  tamperViolationCount = 0;

  tamperNormalCount = 0;

  tamperDate = "";
  tamperTime = "";
}

// =====================================================
// HANDLE BUTTON PRESS
// =====================================================

void handleButtonPress() {
  unsigned long now =
    millis();

  // ---------------------------------------------------
  // QR SCREEN
  // ---------------------------------------------------

  if (qrShowing) {
    qrShowing = false;

    displayNormal();

    Serial.println(
      "QR CLOSED");

    return;
  }

  // ---------------------------------------------------
  // MONITORING MODE
  // ---------------------------------------------------

  if (
    monitoring && referenceCaptured) {
    if (
      pressCount == 0 || now - firstPressTime > TRIPLE_PRESS_WINDOW) {
      pressCount = 1;

      firstPressTime = now;
    } else {
      pressCount++;
    }

    Serial.print(
      "MONITORING BUTTON PRESS: ");

    Serial.println(
      pressCount);

    // -------------------------------------------------
    // THREE CLICKS
    // -------------------------------------------------

    if (
      pressCount >= 3) {
      pressCount = 0;

      showQR();

      return;
    }

    return;
  }

  // ---------------------------------------------------
  // COUNTDOWN
  // ---------------------------------------------------

  if (countdownRunning) {
    return;
  }

  // ---------------------------------------------------
  // WAITING FOR DOUBLE CLICK
  // ---------------------------------------------------

  if (
    pressCount == 0 || now - firstPressTime > DOUBLE_PRESS_WINDOW) {
    pressCount = 1;

    firstPressTime = now;

    Serial.println(
      "FIRST PRESS");

    display.clear();

    display.setFont(
      ArialMT_Plain_10);

    display.drawString(
      0,
      15,
      "FIRST PRESS");

    display.drawString(
      0,
      30,
      "Press again quickly");

    display.display();
  } else {
    pressCount++;

    if (
      pressCount >= 2) {
      Serial.println(
        "DOUBLE PRESS DETECTED");

      pressCount = 0;

      // -----------------------------------------------
      // START CALIBRATION
      // -----------------------------------------------

      countdownRunning = true;

      monitoring = false;

      referenceCaptured = false;

      // -----------------------------------------------
      // CLEAR OLD DATA
      // -----------------------------------------------

      resetTamperHistory();

      // -----------------------------------------------
      // TIMER
      // -----------------------------------------------

      countdownStart = now;

      display.clear();

      display.setFont(
        ArialMT_Plain_10);

      display.drawString(
        0,
        15,
        "CALIBRATION STARTED");

      display.drawString(
        0,
        30,
        "Keep device still");

      display.drawString(
        0,
        45,
        "40 seconds");

      display.display();

      Serial.println(
        "40 SECOND CALIBRATION STARTED");
    }
  }
}

// =====================================================
// READ BUTTON
// =====================================================

void readButton() {
  bool reading =
    digitalRead(
      SWITCH_PIN);

  if (
    reading != buttonLastReading) {
    buttonLastChange =
      millis();

    buttonLastReading =
      reading;
  }

  if (
    millis() - buttonLastChange > BUTTON_DEBOUNCE) {
    if (
      reading != buttonStableState) {
      buttonStableState =
        reading;

      if (
        buttonStableState == LOW) {
        handleButtonPress();
      }
    }
  }
}

// =====================================================
// BUTTON SEQUENCE TIMEOUT
// =====================================================

void checkPressTimeout() {
  if (
    pressCount > 0 && millis() - firstPressTime > DOUBLE_PRESS_WINDOW) {
    pressCount = 0;
  }
}

// =====================================================
// MONITOR SENSORS
// =====================================================

void monitorSensors() {
  if (
    millis() - lastSensorRead < SENSOR_INTERVAL) {
    return;
  }

  lastSensorRead =
    millis();

  readAllSensors();

  bool newTamper =
    checkTamper();

  if (newTamper) {
    displayTamperEvent();
  } else {
    displayNormal();
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(
    115200);

  delay(500);

  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(
    SDA_PIN,
    SCL_PIN);

  // ===================================================
  // BUTTON
  // ===================================================

  pinMode(
    SWITCH_PIN,
    INPUT_PULLUP);

  // ===================================================
  // OLED
  // ===================================================

  setupDisplay();

  display.setFont(
    ArialMT_Plain_10);

  display.drawString(
    0,
    0,
    "MYOSA SENSOR SYSTEM");

  display.drawString(
    0,
    15,
    "MPU6050 : 0x69");

  display.drawString(
    0,
    27,
    "APDS9960: 0x39");

  display.drawString(
    0,
    39,
    "BMP180  : 0x77");

  display.drawString(
    0,
    51,
    "Initializing...");

  display.display();

  delay(500);

  // ===================================================
  // DS3231
  // ===================================================

  initializeRTC();

  // ===================================================
  // MPU6050
  // ===================================================

  Wire.beginTransmission(
    MPU_ADDR);

  Wire.write(
    0x6B);

  Wire.write(
    0x00);

  byte mpuError =
    Wire.endTransmission();

  if (
    mpuError == 0) {
    Serial.println(
      "MPU6050 FOUND");
  } else {
    Serial.print(
      "MPU6050 ERROR: ");

    Serial.println(
      mpuError);
  }

  // ===================================================
  // APDS9960
  // ===================================================

  if (
    !apds.begin()) {
    Serial.println(
      "APDS9960 ERROR!");
  } else {
    Serial.println(
      "APDS9960 FOUND");

    if (
      apds.enableAmbientLightSensor(
        ENABLE)) {
      Serial.println(
        "APDS LIGHT ENABLED");

      // Allow the APDS9960 ambient/RGB integration
      // to settle before the first RGB measurement.
      delay(120);
    }

    if (
      apds.enableProximitySensor(
        ENABLE)) {
      Serial.println(
        "APDS PROXIMITY ENABLED");
    }
  }

  // ===================================================
  // BMP180
  // ===================================================

  if (
    !bmp180.begin()) {
    Serial.println(
      "BMP180 ERROR!");
  } else {
    Serial.println(
      "BMP180 FOUND");
  }

  // ===================================================
  // QR
  // ===================================================

  qrcode.init();

  // ===================================================
  // READY
  // ===================================================

  display.clear();

  display.setFont(
    ArialMT_Plain_10);

  display.drawString(
    0,
    0,
    "SYSTEM READY");

  display.drawString(
    0,
    16,
    "Double click");

  display.drawString(
    0,
    29,
    "Calibration: 40 sec");

  display.drawString(
    0,
    43,
    "3 clicks = QR");

  display.display();

  // ===================================================
  // SERIAL READY MESSAGE
  // ===================================================

  Serial.println();

  Serial.println(
    "================================");

  Serial.println(
    "MYOSA SENSOR SYSTEM READY");

  if (rtcOK) {
    Serial.println(
      "DS3231 RTC READY");
  } else {
    Serial.println(
      "DS3231 RTC ERROR");
  }

  Serial.println(
    "DOUBLE CLICK = START");

  Serial.println(
    "3 CLICKS = QR");

  Serial.println(
    "LOW SENSITIVITY MODE");

  Serial.println(
    "================================");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {
  // ===================================================
  // ALWAYS READ BUTTON
  // ===================================================

  readButton();

  checkPressTimeout();

  // ===================================================
  // QR SCREEN
  // ===================================================

  if (qrShowing) {
    return;
  }

  // ===================================================
  // CALIBRATION
  // ===================================================

  if (countdownRunning) {
    countdownDisplay();

    return;
  }

  // ===================================================
  // MONITORING
  // ===================================================

  if (
    monitoring && referenceCaptured) {
    monitorSensors();

    return;
  }
}