#include <LCD_I2C.h>
#include "OneButton.h"

#define PIN_LED 8
#define PIN_INPUT 2

int etatLcd = 0;
const int X_pin = A1;  // Pin analogique pour l'axe X
const int Y_pin = A2;  // Pin analogique pour l'axe Y
bool ledState = false;
float ThermistorPin = A0;

unsigned long currentTime = 0;
unsigned long previousTime = 0;

// Les tâches pouvant être faites
typedef enum { LCD_PAGE_ONE,
               LCD_PAGE_TWO } Tasks;

OneButton button(PIN_INPUT, true, true);

Tasks currentTask = LCD_PAGE_ONE;

LCD_I2C lcd(0x27, 16, 2);

byte caractPerso[8] = {
  B11100,
  B10000,
  B11100,
  B00111,
  B11101,
  B00111,
  B00101,
  B00111
};
byte degre[8] = {
  B01000,
  B10100,
  B01000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

void setup() {
  lcd.begin();
  lcd.backlight();
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  lcd.createChar(2, degre);
  button.setDebounceTicks(25);
  lcd.createChar(1, caractPerso);
  button.attachClick(myClickFunction);
}

float temperature() {
  int Vo;            // Voltage à la sortie
  float R1 = 10000;  // Résistance
  float logR2, R2, T, temp, Tf;
  float c1 = 1.129148e-03, c2 = 2.34125e-04, c3 = 8.76741e-08;

  const int rate = 50;

  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  temp = T - 273.15;

  return temp;
}

void affichageTemperature() {

  float temp = temperature();

  lcd.setCursor(0, 1);
  lcd.print("Temp.:");

  lcd.setCursor(6, 1);
  lcd.print(temp, 1);

  lcd.setCursor(10, 1);
  lcd.write(byte(2));

  lcd.setCursor(11, 1);
  lcd.print("C");
}

void phare() {

  float temp = temperature();
  short seuilHaut = 25;
  short seuilBas = 24;



  if (temp > seuilHaut) {

    ledState = true;
    digitalWrite(PIN_LED, ledState);

  } else if (temp < seuilBas) {

    ledState = false;
    digitalWrite(PIN_LED, ledState);

  } else {

    digitalWrite(PIN_LED, ledState);
  }
}

void portSerie() {
  int temporisation = 500;
  String numDa = "6334158";
  int valueX_pin = analogRead(X_pin);
  int valueY_pin = analogRead(Y_pin);
  if (currentTime - previousTime >= temporisation) {

    Serial.println("etd:" + numDa + ",x:" + valueX_pin + ",y:" + valueY_pin + "sys:" + ledState);
    previousTime = currentTime;
  }
}

void lcdPhare() {

  float temp = temperature();

  String etat = "OFF";

  if (ledState == true) {

    etat = "ON ";
    lcd.setCursor(0, 0);
    lcd.print("Phare:" + etat);

  } else if (ledState == false) {

    etat = "OFF";
    lcd.setCursor(0, 0);
    lcd.print("Phare:" + etat);


  } else {

    lcd.setCursor(0, 0);
    lcd.print("   ");
    lcd.setCursor(0, 0);
    lcd.print("Phare:" + etat);
  }
}

void joystick() {

  int valueX_pin = analogRead(X_pin);
  int valueY_pin = analogRead(Y_pin);
  int mappedIndexX_pin = map(valueX_pin, 0, 1023, 90, -90);
  int mappedIndexY_pin = map(valueY_pin, 0, 1023, -239, 240);

  String etatX = "Centre";
  String etatY = "Arret";

  if (mappedIndexX_pin > 0) {

    etatX = "Droite";

  } else {

    etatX = "Gauche";
  }

  if (mappedIndexY_pin > 0) {

    etatY = "Avance";

  } else if (mappedIndexY_pin < 0) {

    etatY = "Recule";

  } else {

    etatY = "Arret";
  }

  lcd.setCursor(0, 0);
  lcd.print(etatX + " a ");

  lcd.setCursor(9, 0);
  lcd.print("   ");

  lcd.setCursor(9, 0);
  lcd.print(mappedIndexX_pin);

  lcd.setCursor(12, 0);
  lcd.write(byte(2));

  lcd.setCursor(0, 1);
  lcd.print(etatY);

  lcd.setCursor(13, 1);
  lcd.print("   ");
  lcd.setCursor(8, 1);
  lcd.print(String(mappedIndexY_pin) + "km/h");
}

void demarrage() {

  unsigned long previousTime = 0;
  int rate = 3000;

  if (etatLcd == 0) {

    lcd.setCursor(0, 0);
    lcd.print("Brim");

    lcd.setCursor(0, 1);
    lcd.write(byte(1));

    lcd.setCursor(14, 1);
    lcd.print("58");
  }
  if (currentTime - previousTime >= rate && etatLcd == 0) {
    lcd.clear();
    previousTime = currentTime;
    etatLcd = 1;
  }
}

void pageOne() {

  joystick();
}

void pageTwo() {

  lcdPhare();
  affichageTemperature();
}

void myClickFunction() {

  if (currentTask == LCD_PAGE_ONE) {

    lcd.clear();
    currentTask = LCD_PAGE_TWO;

  } else {

    lcd.clear();
    currentTask = LCD_PAGE_ONE;
  }
}

void loop() {

  currentTime = millis();
  button.tick();
  demarrage();
  phare();
  portSerie();

  if (etatLcd == 1) {

    switch (currentTask) {
      case LCD_PAGE_ONE:

        pageOne();
        break;
      case LCD_PAGE_TWO:

        pageTwo();
        break;
    }
  }
}
