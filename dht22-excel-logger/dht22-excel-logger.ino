/*
  Простое логгирование температуры и влажности в CSV
  Подключение (смотри таблицу):
  - microSD (SPI): CS=D10, MOSI=D11, MISO=D12, SCK=D13, VCC=5V, GND=GND
  - DS3231 (I²C): SDA=A4, SCL=A5, VCC=5V, GND=GND
  - DHT22: DATA=D2, VCC=5V, GND=GND (подтяжка 4.7–10 кОм к VCC)
*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"

#define DHTPIN   2
#define DHTTYPE  DHT22
const uint8_t SD_CS_PIN = 10;

RTC_DS3231 rtc;
DHT dht(DHTPIN, DHTTYPE);

const unsigned long SAMPLE_MS = 5000;      // интервал опроса, мс
const char* FILENAME = "DATA.CSV";         // 8.3 имя файла (надёжно для Nano)
const char  SEP      = ';';                // разделитель столбцов (для Excel-локалей часто удобнее ';')

bool rtc_ok = false;

String twoDigits(uint8_t v) {
  if (v < 10) return String('0') + String(v);
  return String(v);
}

String timestampNow() {
  if (rtc_ok) {
    DateTime now = rtc.now();
    String ts = String(now.year()) + "-" + twoDigits(now.month()) + "-" + twoDigits(now.day());
    ts += " " + twoDigits(now.hour()) + ":" + twoDigits(now.minute()) + ":" + twoDigits(now.second());
    return ts;
  } else {
    // Фолбэк на аптайм устройства (секунды от старта)
    unsigned long tsec = millis() / 1000UL;
    unsigned long s = tsec % 60UL;
    unsigned long m = (tsec / 60UL) % 60UL;
    unsigned long h = (tsec / 3600UL);
    return String("1970-01-01 ") + twoDigits(h % 24) + ":" + twoDigits(m) + ":" + twoDigits(s);
  }
}

bool appendLineToCSV(const String& line) {
  File f = SD.open(FILENAME, FILE_WRITE);
  if (!f) return false;
  f.println(line);
  f.close();          // flush не нужен: close() всё запишет
  return true;
}

void writeHeaderIfEmpty() {
  File f = SD.open(FILENAME, FILE_WRITE);
  if (!f) return;
  if (f.size() == 0) {
    f.print(F("datetime")); f.print(SEP);
    f.print(F("temperature_c")); f.print(SEP);
    f.println(F("humidity_pct"));
  }
  f.close();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(500);

  // DHT
  dht.begin();

  // RTC
  if (rtc.begin()) {
    rtc_ok = true;
    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    rtc_ok = false;
    Serial.println(F("RTC не найден, используем аптайм вместо реального времени."));
  }

  // SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Ошибка инициализации SD. Проверьте CS, питание и модуль."));
    while (true) { digitalWrite(LED_BUILTIN, HIGH); delay(150); digitalWrite(LED_BUILTIN, LOW); delay(150); }
  }

  writeHeaderIfEmpty();
  Serial.println(F("Старт логгера. Пишем в DATA.CSV каждые 5 секунд."));
}

void loop() {
  static unsigned long nextSample = 0;
  unsigned long nowMs = millis();
  if ((long)(nowMs - nextSample) >= 0) {
    nextSample = nowMs + SAMPLE_MS;

    float h = dht.readHumidity();
    float t = dht.readTemperature(); // °C

    if (isnan(h) || isnan(t)) {
      delay(250);
      h = dht.readHumidity();
      t = dht.readTemperature();
    }

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Ошибка чтения DHT22, строка пропущена."));
      return;
    }

    String line;
    line.reserve(48);
    line  = timestampNow();
    line += SEP; line += String(t, 1);
    line += SEP; line += String(h, 1);

    if (!appendLineToCSV(line)) {
      Serial.println(F("Ошибка записи на SD."));
      for (int i = 0; i < 3; i++) { digitalWrite(LED_BUILTIN, HIGH); delay(60); digitalWrite(LED_BUILTIN, LOW); delay(60); }
    } else {
      Serial.println(line);
    }
  }
  delay(5);
}
