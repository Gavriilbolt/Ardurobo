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
    unsigned long tms = millis() / 1000UL;
    unsigned long s = tms % 60UL;
    unsigned long m = (tms / 60UL) % 60UL;
    unsigned long h = (tms / 3600UL);
    // Дата-заглушка + HH:MM:SS от старта
    return String("1970-01-01 ") + twoDigits(h % 24) + ":" + twoDigits(m) + ":" + twoDigits(s);
  }
}

bool appendLineToCSV(const String& line) {
  File f = SD.open(FILENAME, FILE_WRITE);
  if (!f) return false;
  f.println(line);
  bool ok = f.flush(); // на SD библиотеке flush() возвращает void, но оставим для читаемости
  f.close();
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
      // Если батарейка подсела/время сброшено — ставим компиляционное время
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    rtc_ok = false;
    Serial.println(F("RTC не найден, используем аптайм вместо реального времени."));
  }

  // SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Ошибка инициализации SD. Проверьте CS, питание и модуль."));
    // Можем мигать, чтобы визуально видеть фейл
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH); delay(150);
      digitalWrite(LED_BUILTIN, LOW);  delay(150);
    }
  }

  writeHeaderIfEmpty();

  Serial.println(F("Старт логгера. Пишем в DATA.CSV каждые 5 секунд."));
}

void loop() {
  static unsigned long nextSample = 0;
  unsigned long nowMs = millis();
  if ((long)(nowMs - nextSample) >= 0) {
    nextSample = nowMs + SAMPLE_MS;

    // Чтение с DHT22 (библиотеке нужно >=2с между опросами — у нас 5с ок)
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // по умолчанию °C

    // Иногда DHT возвращает NaN — попробуем разок повторить через короткую паузу
    if (isnan(h) || isnan(t)) {
      delay(250);
      h = dht.readHumidity();
      t = dht.readTemperature();
    }

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Ошибка чтения DHT22, строка пропущена."));
      return; // пропускаем запись этой итерации
    }

    // Формируем строку CSV
    String line;
    line.reserve(48);
    line  = timestampNow();
    line += SEP; line += String(t, 1);   // одна цифра после запятой
    line += SEP; line += String(h, 1);

    // Пишем на карту
    if (!appendLineToCSV(line)) {
      Serial.println(F("Ошибка записи на SD."));
      // аварийная индикация
      for (int i = 0; i < 3; i++) { digitalWrite(LED_BUILTIN, HIGH); delay(60); digitalWrite(LED_BUILTIN, LOW); delay(60); }
    } else {
      Serial.println(line);
    }
  }

  // короткий сон, чтобы не жечь CPU вхолостую
  delay(5);
}
