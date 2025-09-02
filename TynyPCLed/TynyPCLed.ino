#include <EEPROM.h>

#define BUTTON_PIN PB2     // Пин кнопки
#define DEBOUNCE_TIME 50   // Время антидребезга, мс

// дефайны настроек, прописываются перед подключением либы
#define TLED_ORDER ORDER_GRB   // порядок цветов
#define TLED_CHIP LED_WS2812   // чип светодиода ленты



#include <FastLEDsupport.h> // вкл поддержку FL
DEFINE_GRADIENT_PALETTE( heatmap_gp ) {   // делаем палитру огня
  0,     0,  0,  0,     // black
  128,   255,  0,  0,   // red
  224,   255, 255,  0,  // bright yellow
  255,   255, 255, 255  // full white
};
CRGBPalette16 fire_p = heatmap_gp;


// либа
#include "tinyLED.h"
tinyLED<3> strip;       // указываем пин (в порядке порта)

#define NUMLEDS 14 // количество светодиодов (для циклов)

volatile bool buttonPressed = false;
unsigned long lastInterruptTime = 0;
byte mode = 0;

void setup() {
  delay(1000);  // Дать стабилизироваться питанию и уровню на пине

  strip.begin(); // <- добавить инициализацию ленты
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  delay(200);  // Дать стабилизироваться питанию и уровню на пине

  attachInterrupt(0, buttonISR, FALLING);

  mode = EEPROM.read(0);  // читаем режим из адреса 0
  if (mode > 23) mode = 23; // проверка на корректность

}



void loop() {
    if (buttonPressed) {
        buttonPressed = false;
        changeMode();
        clearLeds();
      }

    switch (mode) {
    case 0:
      rainbow();
      break;
    case 1:
      rainbowFade();
      break;

    case 2:
      setColor(255, 0, 0); // Красный
      break;
    case 3:
      rule30Color(255, 0, 0);
      break;
    case 4:
      breathingColor(255, 0, 0);
      break;


    case 5:
      setColor(0, 255, 0); // Зелёный
      break;
    case 6:
      rule30Color(0, 255, 0);
      break;
    case 7:
      breathingColor(0, 255, 0);  // Зелёный
      break;

    case 8:
      setColor(0, 0, 255); // Синий
      break;
    case 9:
      breathingColor(0, 0, 255);  // Синий
      break;
    case 10:
      breathingColor(255, 0, 0);
      break;    

    case 11:
      setColor(255, 255, 0); // Жёлтый
      break;
    case 12:
      rule30Color(255, 255, 0);
      break;
    case 13:
      breathingColor(255, 255, 0);  // Жёлтый
      break;
    
    case 14:
      setColor(255, 20, 147); // Розовый
      break;
    case 15:
      rule30Color(255, 20, 147);
      break;
    case 16:
      breathingColor(255, 20, 147);  // Розовый
      break;
    
    case 17:
      setColor(255, 255, 255); // Белый
      break;
    case 18:
      rule30Color(255, 255, 255); 
      break;
    case 19:
      breathingColor(255, 255, 255);  // Белый
      break;
    
    case 20:
      fire(); // перлиновский огонь, лучше изменить
      break;
    case 21:
      randomMarch(); // ярко, красиво, оставляем
      break;
    case 22:
      breathMultiColor();
      break;
    case 23:
      turnOffLeds();// Выключаем подсветку
      break;

  }
}

// Обработчик прерывания
void buttonISR() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
    buttonPressed = true;      // Ставим флаг обработки
    lastInterruptTime = interruptTime;
  }
}

// Функция смены режима
void changeMode() {
  mode++;
  if (mode > 23) mode = 0;
  EEPROM.update(0, mode);  // сохраняем в EEPROM
}

void clearLeds() {
  strip.begin();
  for (int i = 0; i < NUMLEDS; i++) {
    strip.send(0);
  }
  strip.end();
}




// Функция эффекта "бегущая радуга"
void rainbow() {
  static byte count;
  count++;
  strip.begin();
  for (int i = 0; i < NUMLEDS; i++) {
    strip.send(mWheel8(count + i * 255 / NUMLEDS));
  }
  strip.end();
  delay(30);
}

// Функция установки цвета
void setColor(byte r, byte g, byte b) {
    strip.begin();
    for (byte i = 0; i < NUMLEDS; i++) strip.sendRGB(r,g,b);
    strip.end();

    delay(30);
}


// Функция выключения подсветки
void turnOffLeds() {
  for (int i = 0; i < NUMLEDS; i++) {
    strip.send(0); // Выключаем светодиоды
  }
    delay(30);
}

void fire(){
  static int count = 0;
  count += 10;

  strip.begin();
  for (int i = 0; i < NUMLEDS; i++)
    strip.send(CRGBtoData(ColorFromPalette(fire_p, inoise8(i * 25, count), 255, LINEARBLEND)));
  strip.end();
  delay(30);
}

void rainbowFade() { // радуга от цвета к цвету полное заполение 
  static byte ihue = 0;
  ihue++;  // плавный переход по цветам

  strip.begin();  // начать передачу
  for (int i = 0; i < NUMLEDS; i++) {
    // Преобразуем HSV в RGB через встроенную функцию
    strip.send(CRGBtoData(CHSV(ihue, 255, 255)));
  }
  strip.end();  // закончить передачу

  delay(30);
}


void colorBounce() { // типа радар
  static byte idex = 0;
  static bool bouncedirection = false;  // false - вправо, true - влево
  static byte thishue = 0;
  thishue++;  // для динамики цвета

  // смена направления
  if (!bouncedirection) {
    idex++;
    if (idex >= NUMLEDS - 1) {
      bouncedirection = true;
    }
  } else {
    if (idex == 0) {
      bouncedirection = false;
    } else {
      idex--;
    }
  }

  // передача данных в ленту
  strip.begin();
  for (int i = 0; i < NUMLEDS; i++) {
    if (i == idex) {
      strip.send(CRGBtoData(CHSV(thishue, 255, 255)));
    } else {
      strip.send(0);  // выключен
    }
  }
  strip.end();

  delay(30);
}

void rule30Color(byte r, byte g, byte b) {
  static bool initialized = false;
  static byte ledsState[NUMLEDS];        // 0 или 1 (вкл/выкл)
  static byte ledsStatePrev[NUMLEDS];

  if (!initialized) {
    for (int i = 0; i < NUMLEDS; i++) {
      ledsState[i] = random(2);  // 0 или 1
    }
    initialized = true;
  }

  // копируем в предыдущее поколение
  for (int i = 0; i < NUMLEDS; i++) {
    ledsStatePrev[i] = ledsState[i];
  }

  // применяем правило 30
  for (int i = 0; i < NUMLEDS; i++) {
    byte left  = ledsStatePrev[(i - 1 + NUMLEDS) % NUMLEDS];
    byte self  = ledsStatePrev[i];
    byte right = ledsStatePrev[(i + 1) % NUMLEDS];

    byte pattern = (left << 2) | (self << 1) | right;

    switch (pattern) {
      case 0b100:
      case 0b011:
      case 0b010:
      case 0b001:
        ledsState[i] = 1;
        break;
      default:
        ledsState[i] = 0;
        break;
    }
  }

  // выводим результат
  strip.begin();
  for (int i = 0; i < NUMLEDS; i++) {
    if (ledsState[i]) {
      strip.sendRGB(r, g, b);
    } else {
      strip.send(0);  // выключено
    }
  }
  strip.end();

  delay(150);
}


void randomMarch() {
  static CRGB ledsBuf[NUMLEDS];      // текущие цвета
  static CRGB ledsPrev[NUMLEDS];     // предыдущее поколение

  // сохраняем предыдущее состояние
  for (int i = 0; i < NUMLEDS; i++) {
    ledsPrev[i] = ledsBuf[i];
  }

  // первый светодиод получает случайный цвет
  ledsBuf[0] = CHSV(random(0, 255), 255, 255);

  // все остальные получают цвет от соседа CCW (следующего по индексу)
  for (int i = 1; i < NUMLEDS; i++) {
    int iCCW = (i + 1) % NUMLEDS;
    ledsBuf[i] = ledsPrev[iCCW];
  }

  // выводим результат
  strip.begin();
  for (int i = 0; i < NUMLEDS; i++) {
    strip.send(CRGBtoData(ledsBuf[i]));
  }
  strip.end();

  delay(100);  // можно регулировать скорость
}

void breathMultiColor() {
  static uint8_t brightness = 0;
  static int8_t delta = 5; // шаг изменения яркости
  static uint8_t colorIndex = 0;
  static const CRGB colors[] = {
    CRGB(255, 0, 0),     // Красный
    CRGB(0, 255, 0),     // Зелёный
    CRGB(0, 0, 255),     // Синий
    CRGB(255, 255, 0),   // Жёлтый
    CRGB(255, 20, 147),  // Розовый
    CRGB(255, 255, 255)  // Белый
  };

  brightness += delta;

  // когда достигли максимума — начинаем убывание
  if (brightness == 0 || brightness == 255) {
    delta = -delta;

    // при полном "выдохе" меняем цвет
    if (brightness == 0) {
      colorIndex++;
      if (colorIndex >= sizeof(colors) / sizeof(colors[0])) {
        colorIndex = 0;
      }
    }
  }

  CRGB currentColor = colors[colorIndex];
  uint8_t r = (currentColor.r * brightness) / 255;
  uint8_t g = (currentColor.g * brightness) / 255;
  uint8_t b = (currentColor.b * brightness) / 255;

  strip.begin();
  for (byte i = 0; i < NUMLEDS; i++) {
    strip.sendRGB(r, g, b);
  }
  strip.end();

  delay(30); // регулирует скорость дыхания
}

void breathingColor(uint8_t r_base, uint8_t g_base, uint8_t b_base) {
  static uint8_t brightness = 0;
  static int8_t delta = 5;

  brightness += delta;

  if (brightness == 0 || brightness == 255) {
    delta = -delta;
  }

  uint8_t r = (r_base * brightness) / 255;
  uint8_t g = (g_base * brightness) / 255;
  uint8_t b = (b_base * brightness) / 255;

  strip.begin();
  for (byte i = 0; i < NUMLEDS; i++) {
    strip.sendRGB(r, g, b);
  }
  strip.end();

  delay(50);
}



