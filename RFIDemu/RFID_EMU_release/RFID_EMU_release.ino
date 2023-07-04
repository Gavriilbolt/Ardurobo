#include <EncButton.h>
EncButton<EB_TICK, VIRT_BTN> UP_BTN;
EncButton<EB_TICK, VIRT_BTN> SELECT_BTN;
EncButton<EB_TICK, VIRT_BTN> DOWN_BTN;


#include <AnalogKey.h>
int16_t sigs[3] = {
  1023, 601, 299
  };
  
// указываем пин, количество кнопок и массив значений
AnalogKey<A0, 3, sigs> keys;
// инициализируем антену
#define ANTENNA 2

// всё что касается ключей 
uint32_t cardIDs[] = {
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000,
  0x0000000000
};
volatile int bit_counter=0;
volatile int byte_counter=0;
volatile int half=0;

uint8_t data[8];

// решено использовать библиотеку U8glib. Она позволяет перевернуть дисплей.
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 

// ***НАСТРОЙКИ МЕНЮ РИДЕРА***
#define MENU_ITEMS 13  // Количество пунктов меню
#define MENU_WIDTH 64  // Ширина меню в пикселях
#define MENU_HEIGHT 10  // Высота каждого пункта меню в пикселях

// Массив с названиями пунктов меню (НОМЕР ПУНКТА + НАЗВАНИЕ ИЗ 6-7 СИМВОЛОВ)
const char* menuItems[MENU_ITEMS] = {
  "1. Emty",
  "2. Emty",
  "3. Emty",
  "4. Emty",
  "5. Emty",
  "6. Emty",
  "7. Emty",
  "8. Emty",
  "9. Emty",
  "10. Emty",
  "11. Emty",
  "12. Emty",
  "13. Emty"
};


int selectedMenuItem = 0;  // Выбранный пункт меню

void data_card_ul() {
  uint64_t card_id = (uint64_t)cardIDs[selectedMenuItem];
  uint64_t data_card_ul = (uint64_t)0x1FFF; //first 9 bit as 1
  int32_t i;
  uint8_t tmp_nybble;
  uint8_t column_parity_bits = 0;
  for (i = 9; i >= 0; i--) { //5 bytes = 10 nybbles
    tmp_nybble = (uint8_t) (0x0f & (card_id >> i*4));
    data_card_ul = (data_card_ul << 4) | tmp_nybble;
    data_card_ul = (data_card_ul << 1) | ((tmp_nybble >> 3 & 0x01) ^ (tmp_nybble >> 2 & 0x01) ^\
      (tmp_nybble >> 1 & 0x01) ^ (tmp_nybble  & 0x01));
    column_parity_bits ^= tmp_nybble;
  }
  data_card_ul = (data_card_ul << 4) | column_parity_bits;
  data_card_ul = (data_card_ul << 1); //1 stop bit = 0
  for (i = 0; i < 8; i++) {
    data[i] = (uint8_t)(0xFF & (data_card_ul >> (7 - i) * 8));
  }
}


void setupTimer1() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 4095;
  TCCR1B |= (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}


void drawMenu() {
  u8g.setFont(u8g_font_6x10);  // Шрифт для текста меню

  for (int i = 0; i < MENU_ITEMS; i++) {
    // Отображение каждого пункта меню
    if (i == selectedMenuItem) { // Выделение выбранного пункта
      u8g.drawBox(0, i * MENU_HEIGHT, MENU_WIDTH, MENU_HEIGHT);  
      u8g.setColorIndex(0);  // Цвет текста выбранного пункта
    } else {
      u8g.setColorIndex(1);  // Цвет текста остальных пунктов
    }
    // Отображение текста пункта меню
    u8g.drawStr(2, (i+1) * MENU_HEIGHT - 3, menuItems[i]);  
  }
}


void draw(void) {
  u8g.firstPage();
  do {
     Serial.println("""рисуем""");
    drawMenu();  // Отображение меню
  } while (u8g.nextPage());
}


void setup() {
  u8g.setRot270(); // переворачиваем дисплей как нам нужно
  draw(); // отрисовываем первый раз, чтобы небыло артифактов
  Serial.begin(9600);
  pinMode(ANTENNA, OUTPUT); // определяем антенну    

}

void loop() {
  UP_BTN.tick(keys.status(0));
  SELECT_BTN.tick(keys.status(1));
  DOWN_BTN.tick(keys.status(2));
 
  if (UP_BTN.click()){
    selectedMenuItem--;
    if (selectedMenuItem < 0) {
      selectedMenuItem = MENU_ITEMS - 1;
    }
    draw();
  } else if (DOWN_BTN.click()){
    selectedMenuItem++;
    if (selectedMenuItem >= MENU_ITEMS) {
      selectedMenuItem = 0;
    }
    draw();
  }
  
  if (SELECT_BTN.click()){ 
    Serial.println(selectedMenuItem);
    data_card_ul();  
    setupTimer1();
  }
}

ISR(TIMER1_COMPA_vect) {
        TCNT1=0;
	if (((data[byte_counter] << bit_counter)&0x80)==0x00) {
	    if (half==0) digitalWrite(ANTENNA, LOW);
	    if (half==1) digitalWrite(ANTENNA, HIGH);
	}
	else {
	    if (half==0) digitalWrite(ANTENNA, HIGH);
	    if (half==1) digitalWrite(ANTENNA, LOW);
	}
    
	half++;
	if (half==2) {
	    half=0;
	    bit_counter++;
	    if (bit_counter==8) {
	        bit_counter=0;
	        byte_counter=(byte_counter+1)%8;
		}
	}
}
