#include <EncButton.h>
#include <AnalogKey.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <SPI.h>
#define KB_PIN 0

Adafruit_PCD8544 display = Adafruit_PCD8544(13, 12, 11, 10, 9);
boolean dl = false, dr = false, du = false, dd = false;
int x[50], y[50], i, slength, tempx = 10, tempy = 10, xx, yy;
unsigned int high;
uint8_t bh, bl;
int xegg, yegg;
int freq, tb;
unsigned long time = 280, beeptime = 50;
int score = 0, flag = 0;

EncButton<EB_TICK, VIRT_BTN> btnRight;
EncButton<EB_TICK, VIRT_BTN> btnLeft;
EncButton<EB_TICK, VIRT_BTN> btnUp;
EncButton<EB_TICK, VIRT_BTN> btnDown;

// создаём массив значений сигналов с кнопок
  int16_t sigs[4] = {
    393, 732, 
    1023, 538
  };

// указываем пин, количество кнопок и массив значений
  AnalogKey<KB_PIN, 4, sigs> keys;

void setup(){
  Serial.begin(9600);
  display.begin();
  display.setContrast(50);
  display.clearDisplay();
  display.drawRoundRect(0, 0, 84 , 25, 1, 2);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(12, 6);
  display.println("SNAKE");
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.display();
  delay(5000);
  slength = 8;
  xegg = (display.width()) / 2;
  yegg = (display.height()) / 2;
  display.clearDisplay();
  display.drawRect(0, 0, 84, 48, 1);
  display.setCursor(4, 2);
  display.print("S:     R:");//надпись score и record
  display.setCursor(22, 2);
  display.print(score);
  display.setCursor(64, 2);
  display.print(high);
  display.drawRect(0, 0, 84, 11, 1);
  //координаты
  for (i = 0; i <= slength; i++)
  {
    x[i] = 25 - 3 * i;
    y[i] = 30;
  }
  for (i = 0; i < slength; i++)  //Рисуем змею
  {
    display.drawCircle(x[i], y[i], 1, BLACK);
  }
  display.display();
  dr = true;
}

void loop()
{
  movesnake();
}
 
void movesnake()
{
 
  if (flag == 0)
  {
    direct();
  }
 
  if (millis() % time == 0)
  {
    if (flag == 0)
    {
      if (dr == true) {
        tempx = x[0] + 3;
        tempy = y[0];
      }
      if (dl == true) {
        tempx = x[0] - 3;
        tempy = y[0];
      }
      if (du == true) {
        tempy = y[0] - 3;
        tempx = x[0];
      }
      if (dd == true) {
        tempy = y[0] + 3;
        tempx = x[0];
      }
    }
    flag = 0;
    checkgame();
    checkegg();
 
    //изменение координат
    for (i = 0; i <= slength; i++)
    {
      xx = x[i];
      yy = y[i];
      x[i] = tempx;
      y[i] = tempy;
      tempx = xx;
      tempy = yy;
    }
    //перерисовка змейки и цели
    drawsnake();
  }
}
 
void checkgame()
{
  for (i = 1; i < slength; i++)
  {
    //Проверяет, что рекорд побит и вывод нового результата
    high = (((0xff00 + bh) << 8) + bl);
    if (score > high)
    {
      high = score;
      bh = (high >> 8);
      bl = high & 0xff;
      display.fillRect(61, 1, 20, 9, 0);
      display.setCursor(65, 2);
      display.print(high);
    }
    //Проверка касания границ игрового поля
    if ((x[0] <= 1 || x[0] >= 83) || (y[0] <= 12 || y[0] >= 47) || (x[i] == x[0] && y[i] == y[0]) )
    {
      //Если коснулась, то проигрыш. Выводит результаты
      display.clearDisplay();
      display.fillRoundRect(0, 0, 84 , 31, 1, 2);
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.setCursor(18, 1);
      display.print("GAME");
      display.setCursor(18, 16);
      display.print("OVER");
      display.setTextColor(BLACK);
      display.setTextSize(1);
      display.setCursor(12, 33);
      display.print("SCORE");
      display.setCursor(60, 33);
      display.print(score);
      display.setCursor(12, 41);
      display.print("RECORD");
      display.setCursor(60, 41);
      display.print(high);
      display.display();
      //ждем 5 сек и перезапускаем игру
      delay(5000);
 
      //очищаем дисплей
      display.clearDisplay();
      //возвращаем к исходному положению
      slength = 8;
      score = 0;
      time = 280;
      redraw();
    }
  }
}
 
void checkegg()      //змейка ест добычу
{
  //Проверяем что у змеи и добычи одни и те же координаты
  if (x[0] == xegg or x[0] == (xegg + 1) or x[0] == (xegg + 2) or x[0] == (xegg - 1))
  {
    if (y[0] == yegg or y[0] == (yegg + 1) or y[0] == (yegg + 2) or y[0] == (yegg - 1))
    {
      //змея увеличивается, значение очков плюсуем
      score += 1;
      display.fillRect(21, 1, 20, 9, 0);
      display.setCursor(22, 2);
      display.print(score);
      slength += 1;
      if (time >= 90)
      {
        time -= 10;
      }
      display.fillRect(xegg, yegg, 3, 3, WHITE);
      display.display();
    
      //перерисовываем добычу в новом месте
      xegg = random(2, 80);
      yegg = random(15, 40);
    }
  }
}
 
void direct()
{
  btnRight.tick(keys.status(0));
  btnLeft.tick(keys.status(1));
  btnUp.tick(keys.status(2));
  btnDown.tick(keys.status(3));

  //изменение движения если нажимаем на кнопки
  if (btnRight.click() and dr == false) //право
  {
    dl = true; du = false; dd = false;   
    tempx = x[0] - 3;
    tempy = y[0];
    flag = 1;
  }
  else if (btnLeft.click() and dl == false)//лево
  {
    dr = true; du = false; dd = false;
    tempx = x[0] + 3;
    tempy = y[0];
    flag = 1;
  }
  else if (btnUp.click() and dd == false)// вверх
  {
    du = true; dl = false; dr = false;
    tempy = y[0] - 3;
    tempx = x[0];
    flag = 1;
  }
  else if (btnDown.click() and du == false) //вниз
  {
    dd = true; dl = false; dr = false;
    tempy = y[0] + 3;
    tempx = x[0];
    flag = 1;
  }

}
 
void drawsnake()
{
  display.fillRect(xegg, yegg, 3, 3, BLACK);
  display.drawCircle(x[0], y[0], 1, BLACK);
  display.drawCircle(x[slength], y[slength], 1, WHITE);
  display.display();
}
 
void redraw()
{
  display.drawRect(0, 0, 84, 48, 1);
  display.drawRect(0, 0, 84, 48, 1);
  display.setCursor(4, 2);
  display.print("S:     R:");
  display.drawRect(0, 0, 84, 11, 1);
  display.fillRect(21, 1, 20, 9, 0);
  display.setCursor(22, 2);
  display.print(score);
  display.fillRect(61, 1, 20, 9, 0);
  display.setCursor(65, 2);
  display.print(high);
 
  xegg = (display.width()) / 2;
  yegg = (display.height()) / 2;
  dl = false, dr = false, du = false, dd = false;
  dr = true;
  display.setCursor(4, 2);
  display.print("S:     R:");
  display.drawRect(0, 0, 84, 11, 1);
  //возвращаем начальные координаты
  for (i = 0; i <= slength; i++)
  {
    x[i] = 25 - 3 * i;
    y[i] = 30;
  }
  tempx = 33 - 3 * i;
  tempy = 30;
  display.display();
}
