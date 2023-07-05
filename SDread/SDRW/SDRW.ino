#include <SPI.h>
#include <SD.h>

const int chipSelect = 10; // Пин выбора для карты SD

// Параметры светодиодной ленты
const int numLEDs = 10; // Общее количество светодиодов
byte brightness = 255; // Яркость светодиодов (0-255)

// Параметры цвета
byte red = 255; // Красный (0-255)
byte green = 0; // Зеленый (0-255)
byte blue = 0; // Синий (0-255)

void setup() {
  Serial.begin(9600);
  
  // Инициализация карты SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Ошибка инициализации карты SD!");
    return;
  }
  
  Serial.println("Карта SD успешно инициализирована.");
}

void loop() {
  // Чтение параметров с последовательного порта
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    processInput(input);
  }
}

void processInput(String input) {
  // Разделение строки на параметры
  int separatorIndex = input.indexOf(',');
  if (separatorIndex == -1) {
    Serial.println("Некорректный ввод!");
    return;
  }
  
  // Извлечение параметров из строки
  red = input.substring(0, separatorIndex).toInt();
  input.remove(0, separatorIndex + 1);
  
  separatorIndex = input.indexOf(',');
  if (separatorIndex == -1) {
    Serial.println("Некорректный ввод!");
    return;
  }
  
  green = input.substring(0, separatorIndex).toInt();
  input.remove(0, separatorIndex + 1);
  
  blue = input.toInt();
  
  // Запись параметров на карту SD
  writeParametersToSD();
  
  // Вывод параметров в последовательный порт
  Serial.print("Получены новые параметры: R=");
  Serial.print(red);
  Serial.print(", G=");
  Serial.print(green);
  Serial.print(", B=");
  Serial.println(blue);
}

void writeParametersToSD() {
  // Открываем файл на запись
  File dataFile = SD.open("parameters.txt", FILE_WRITE);
  
  if (dataFile) {
    // Записываем параметры в файл
    dataFile.print("R=");
    dataFile.println(red);
    dataFile.print("G=");
    dataFile.println(green);
    dataFile.print("B=");
    dataFile.println(blue);
    
    // Закрываем файл
    dataFile.close();
    
    Serial.println("Параметры успешно записаны на карту SD.");
  }
  else {
    Serial.println("Ошибка открытия файла на запись!");
  }
}
