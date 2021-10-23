#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

int SS_PIN = 10;  //(rfid)
int RST_PIN = 9;  // 50miso 51mosi 52sck  (rfid) / 12miso 11mosi 13sck A5 SCL - A4 -SDA

int CE_PIN = 7;
int CSN_PIN = 8;
RF24 rf(CE_PIN, CSN_PIN);

int total = 0; //basket total
int water = 2; //tl
int chips = 6; //tl
int milk = 8; //tl
int yoghurt = 9; //tl
int water_piece, chips_piece, milk_piece, yoghurt_piece;
int remove_button = A1; //remove push button pin
int buzzer = 2;

TinyGPS gps;
SoftwareSerial ss(5, 6); //rx,tx

float flat, flon;
unsigned long age;

LiquidCrystal_I2C lcd(0x27, 20, 4); //20x4 LCD define.
MFRC522 rfid(SS_PIN, RST_PIN);  //rfid defined pins.

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd_start();
  total = 0;
  pinMode(remove_button, INPUT_PULLUP);
  int water_piece = 0;
  int chips_piece = 0;
  int milk_piece = 0;
  int yoghurt_piece = 0;
  int auto_counter = 0;
  int manuel_counter = 0;
  pinMode(buzzer, OUTPUT);
  rf.begin();
  rf.openWritingPipe(address);
  rf.setPALevel(RF24_PA_MAX);
  rf.stopListening();
  ss.begin(9600);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void loop() {
  smartdelay(1000);
  uint8_t sat = gps.satellites();
  gps.f_get_position(&flat, &flon, &age);
  int remove_int = analogRead(remove_button);
  char coordinate[100] ;
  sprintf(coordinate, "%d.%ld,%d.%ld", (int)flat, long(flat * 1000000 - int(flat) * 1000000), (int)flon, long(flon * 1000000 - int(flon) * 1000000));
  rf.write(&coordinate, sizeof(coordinate));
  if (remove_int >= 950) {
    lcd_standby();
    if ( ! rfid.PICC_IsNewCardPresent())
      return;
    if ( ! rfid.PICC_ReadCardSerial())
      return;
    if (rfid.uid.uidByte[0] == 186 &&
        rfid.uid.uidByte[1] == 117 &&
        rfid.uid.uidByte[2] == 102 &&
        rfid.uid.uidByte[3] == 103 ) {
      total = total + milk;
      milk_piece = milk_piece + 1;
      buzzer_tone();
      //Serial.print("m");
      lcd_milk();
    }
    else if (rfid.uid.uidByte[0] == 178 &&
             rfid.uid.uidByte[1] == 172 &&
             rfid.uid.uidByte[2] == 178 &&
             rfid.uid.uidByte[3] == 49 ) {

      total = total + chips;
      chips_piece = chips_piece + 1;
      buzzer_tone();
      //Serial.print("c");
      lcd_chips();
    }
    else if (rfid.uid.uidByte[0] == 217 &&
             rfid.uid.uidByte[1] == 105 &&
             rfid.uid.uidByte[2] == 28 &&
             rfid.uid.uidByte[3] == 179 ) {

      total = total + water;
      water_piece = water_piece + 1;
      buzzer_tone();
      //Serial.print("w");
      lcd_water();
    }
    else if (rfid.uid.uidByte[0] == 153 &&
             rfid.uid.uidByte[1] == 93 &&
             rfid.uid.uidByte[2] == 241 &&
             rfid.uid.uidByte[3] == 178 ) {

      total = total + yoghurt;
      yoghurt_piece = yoghurt_piece + 1;
      buzzer_tone();
      //Serial.print("y");
      lcd_yoghurt();
    }
    else {
      undefined();
    }
    rfid.PICC_HaltA();
  }
  else if (remove_int <= 150) {
    lcd_remove();
    if ( ! rfid.PICC_IsNewCardPresent())
      return;
    if ( ! rfid.PICC_ReadCardSerial())
      return;
    if (rfid.uid.uidByte[0] == 186 &&
        rfid.uid.uidByte[1] == 117 &&
        rfid.uid.uidByte[2] == 102 &&
        rfid.uid.uidByte[3] == 103 ) {
      if (milk_piece >= 1) {
        total = total - milk;
        //Serial.print("k");
        buzzer_tone();
        rlcd_milk();
        milk_piece = milk_piece - 1;
      }
    }
    else if (rfid.uid.uidByte[0] == 178 &&
             rfid.uid.uidByte[1] == 172 &&
             rfid.uid.uidByte[2] == 178 &&
             rfid.uid.uidByte[3] == 49 ) {
      if (chips_piece >= 1) {
        total = total - chips;
        //Serial.print("f");
        buzzer_tone();
        rlcd_chips();
        chips_piece = chips_piece - 1;
      }
    }
    else if (rfid.uid.uidByte[0] == 217 &&
             rfid.uid.uidByte[1] == 105 &&
             rfid.uid.uidByte[2] == 28 &&
             rfid.uid.uidByte[3] == 179 ) {
      if (water_piece >= 1) {
        total = total - water;
        //Serial.print("s");
        buzzer_tone();
        rlcd_water();
        water_piece = water_piece - 1;
      }
    }
    else if (rfid.uid.uidByte[0] == 153 &&
             rfid.uid.uidByte[1] == 93 &&
             rfid.uid.uidByte[2] == 241 &&
             rfid.uid.uidByte[3] == 178 ) {
      if (yoghurt_piece >= 1) {
        total = total - yoghurt;
        //Serial.print("h");
        buzzer_tone();
        rlcd_yoghurt();
        yoghurt_piece = yoghurt_piece - 1;
      }
    }
    else {
      undefined();
    }
    rfid.PICC_HaltA();
  }
  char data[100];
  sprintf(data, "%d,%d,%d,%d,%d", int(total), int(yoghurt_piece), int(water_piece), int(milk_piece), int(chips_piece));
  Serial.println(data);
  //Serial.println(total + "," + int(yoghurt_piece) + "," + int(water_piece) + "," + int(milk_piece) + "," + int(chips_piece));
  /*Serial.print(",");
    Serial.print(yoghurt_piece);
    Serial.print(",");
    Serial.print(water_piece);
    Serial.print(",");
    Serial.print(milk_piece);
    Serial.print(",");
    Serial.println(chips_piece);*/
}

void lcd_start() {
  lcd.begin();
  lcd.clear();
  delay(1000);
  lcd.setCursor(5, 1);
  lcd.print("Welcome to");
  lcd.setCursor(7, 2);
  lcd.print("E-Cart");
  delay(1500);
  lcd.clear();
}

void lcd_remove() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Plaese read the");
  lcd.setCursor(0, 1);
  lcd.print("Product for Removing");
  lcd.setCursor(4, 2);
  lcd.print("from Basket");
  delay(200);
  lcd.clear();


  /*lcd.print("Basket Total");
    lcd.setCursor(8,2);
    lcd.print(total);
    lcd.setCursor(11,2);
    lcd.print("TL"); */
}

void lcd_milk() {
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("Added Milk");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}

void lcd_chips() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Added Chips");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}
void lcd_standby() {

  /*lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Plaese read the");
    lcd.setCursor(6, 1);
    lcd.print("Product");
    lcd.setCursor(1, 3);
    lcd.print("Basket Total=");
    lcd.setCursor(14, 3);
    lcd.print(total);
    lcd.setCursor(18, 3);
    lcd.print("TL");
    delay(600); */

  lcd.setCursor(0, 0);
  lcd.print("Milk");
  lcd.setCursor(5, 0);
  lcd.print("x");
  lcd.setCursor(6, 0);
  lcd.print(milk_piece);
  lcd.setCursor(11, 0);
  lcd.print("Water");
  lcd.setCursor(17, 0);
  lcd.print("x");
  lcd.setCursor(18, 0);
  lcd.print(water_piece);
  lcd.setCursor(0, 1);
  lcd.print("Yoghurt");
  lcd.setCursor(8, 1);
  lcd.print("x");
  lcd.setCursor(9, 1);
  lcd.print(yoghurt_piece);
  lcd.setCursor(11, 1);
  lcd.print("Chips");
  lcd.setCursor(17, 1);
  lcd.print("x");
  lcd.setCursor(18, 1);
  lcd.print(chips_piece);
  lcd.setCursor(0, 3);
  lcd.print("TOTAL=");
  lcd.setCursor(6, 3);
  lcd.print(total);
  lcd.setCursor(9, 3);
  lcd.print("TL");
  delay(25);
}
void no_bluetooth() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("NO BLUETOOTH");
  lcd.setCursor(5, 2);
  lcd.print("CONNECTION");
  delay(900);
}
void undefined() {
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("Undefined");
  lcd.setCursor(6, 2);
  lcd.print("Product");
  delay(1500);
  lcd.clear();
}

void lcd_water() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Added Water");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}

void lcd_yoghurt() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Added Yoghurt");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}

void rlcd_chips() {
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("Removed Chips");
  lcd.setCursor(4, 2);
  lcd.print("from Basket");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}

void rlcd_milk() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Removed Milk");
  lcd.setCursor(4, 2);
  lcd.print("from Basket");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}
void rlcd_water() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Removed Water");
  lcd.setCursor(4, 2);
  lcd.print("from Basket");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}

void rlcd_yoghurt() {
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Removed Yoghurt");
  lcd.setCursor(4, 2);
  lcd.print("from Basket");
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Basket Total");
  lcd.setCursor(8, 2);
  lcd.print(total);
  lcd.setCursor(12, 2);
  lcd.print("TL");
  delay(1500);
  lcd.clear();
}

void buzzer_tone() {
  tone(buzzer, 500);
  delay(80);
  noTone(buzzer);
}
