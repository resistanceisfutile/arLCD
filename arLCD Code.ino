#include <ezLCDLib.h>
#include <stdlib.h>


#include <EEPROM.h>
#include <cstlib>
#include <Adafruit_CC3000.h>
#include <ccspi.uh>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h" 

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "MIT GUEST"           // cannot be longer than 32 characters!
#define WLAN_PASS       "myPassword"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_UNSEC

#define IDLE_TIMEOUT_MS  1000      // Amount of time to wait (in milliseconds) with no data
// received before closing the connection.  If you know the server
// you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "rif.herokuapp.com"
#define WEBPAGE      "/empty/"

#define R1 0
#define R2 1
#define R3 2
#define R4 3
#define R5 4

uint8_t sol1 = A1;
uint8_t sol2 = A2;

int value, address;
uint32_t ip;

// start reading from the first byte (address 0) of the EEPROM
int resistor_qty[5] = {
  0, 0, 0, 0, 0};

int dispense_qty[5] = {
  0, 0, 0, 0, 0};


void get_qty();
void store_qty();
void add_qty(int rnum, int amount);
void set_qty(int rnum, int amount);
int take_qty(int rnum, int amount);

ezLCD3 lcd;

volatile int state = 0;
volatile int touch = 0;
boolean didUpdateView = false;

#define button_width 250
#define button_height 30

#define xInset 40

#define yInset 20
void setup()
{ 
  
    pinMode(sol1,OUTPUT);
  pinMode(sol2,OUTPUT);
//  init_wifi();
  //Initialize LCD
  lcd.begin(EZM_BAUD_RATE);
  lcd.string(1, "Left Align");
  state = 0;
  //Theme for buttons
  lcd.theme( 1, WHITE, 3, WHITE, 0, 0, RED, WHITE, 0, RED, 1 );

  //Strings
  lcd.string(1, "Dispense Resistor");

  //Ohms
  lcd.string(2, "100ohm");
  lcd.string(3, "1kohm");
  lcd.string(4, "10kohm");
  lcd.string(5, "100kohm");
  lcd.string(6, "1Mohm");
  lcd.string(7, "+");
  lcd.string(8, "-");
  lcd.string(9, "Done");
  lcd.string(10, "Cancel");
  lcd.string(11, "Update Resistor");
  lcd.string(12, "About");
  get_qty();
}


void loop()
{
  
//  moveServo();
  if(!didUpdateView)
  {
    lcd.cls(BLACK, WHITE);
    switch(state)
    {
    case 0:
      {
//        lcd.picture(0,0,"resistor.gif");
        lcd.button(1, xInset, yInset, button_width, button_height, 1, 4, 20, 1, 1);
        lcd.button(14, xInset, yInset + button_height + 40, button_width, button_height, 1, 4, 20, 1, 11);
        lcd.button(27, xInset, yInset + button_height*2 + 40*2, button_width, button_height, 1, 4, 20, 1, 12);
      }
      break;
    case 1:
      {

        int increment  = 35;
        int yTop = 10;
        lcd.xy(20, yTop);
        lcd.printString("R");
        lcd.xy(80, yTop);
        lcd.printString("Rem.");
        lcd.xy(150, yTop);
        lcd.printString("Dispense");

        int a = 2;
        for(int i = 0, initialY = 40; i < 5; i ++, initialY+= increment)
        {
          char * name;
          switch(i)
          {
          case 0: 
            name = "100";
            break;
          case 1: 
            name = "1k"; 
            break;
          case 2: 
            name = "10k"; 
            break;
          case 3: 
            name = "100k"; 
            break;
          case 4: 
            name = "1M"; 
            break;
          default: 
            break;
          }

          lcd.xy(20, initialY);
          lcd.printString(name);

          char remaining [5];
          itoa(resistor_qty[i], remaining, 10);

          lcd.xy(80, initialY);
          lcd.printString(remaining);

          char dispenseStr [5];
          itoa(dispense_qty[i], dispenseStr, 10);

          lcd.xy(150, initialY);
          lcd.printString(dispenseStr);


          lcd.button(a + i * 2, 220, initialY-10, 35, 35, 1, 4, 20, 1, 8);
          lcd.button(a + i * 2 + 1, 260, initialY-10, 35, 35, 1, 4, 20, 1, 7);
        }
        lcd.button(13, 20, 210, 80, 25, 1, 4, 1,1, 10);
        lcd.button(12, 220 , 210, 80, 25, 1,4,1,1,9);
      }
      break;
    case 2:
      {
        int increment  = 35;
        int yTop = 10;
        lcd.xy(20, yTop);
        lcd.printString("R");
        lcd.xy(80, yTop);
        lcd.printString("Rem.");
        lcd.xy(150, yTop);
        lcd.printString("Update");

        int a = 15;
        for(int i = 0, initialY = 40; i < 5; i ++, initialY+= increment)
        {
          char * name;
          switch(i)
          {
          case 0: 
            name = "100";
            break;
          case 1: 
            name = "1k"; 
            break;
          case 2: 
            name = "10k"; 
            break;
          case 3: 
            name = "100k"; 
            break;
          case 4: 
            name = "1M"; 
            break;
          default: 
            break;
          }

          lcd.xy(20, initialY);
          lcd.printString(name);

          char remaining [5];
          itoa(resistor_qty[i], remaining, 10);

          lcd.xy(80, initialY);
          lcd.printString(remaining);

          char dispenseStr [5];
          itoa(dispense_qty[i], dispenseStr, 10);

          lcd.xy(150, initialY);
          lcd.printString(dispenseStr);


          lcd.button(a + i * 2, 220, initialY-10, 35, 35, 1, 4, 20, 1, 8);
          lcd.button(a + i * 2 + 1, 260, initialY-10, 35, 35, 1, 4, 20, 1, 7);
        }
        lcd.button(25, 20, 210, 80, 25, 1, 4, 1,1, 10);
        lcd.button(26, 220 , 210, 80, 25, 1,4,1,1,9);

      }
      break;
    case 3:
      {
        lcd.xy(20 , 20);
        lcd.printString("Created By:");
        lcd.button(28, 220 , 210, 80, 25, 1,4,1,1,9);
        
         lcd.xy(50 , 50);
        lcd.printString("Timothy Chong");
        lcd.xy(50 , 80);
        lcd.printString("Dean A. De Carli");
        lcd.xy(50, 110);
        lcd.printString("Christorpher J. Woodall");
        lcd.xy(50, 140);
        lcd.printString("John Aleman");
        lcd.xy(50, 170);
        lcd.printString("Andrew Trainor");

      }

      break;
    } 
    didUpdateView = true;
  }

  switch(state)
  {
  case 0:
    {
      lcd.wstack(0);
      if(lcd.currentWidget == 1)
      {        
        //memset(dispense_qty, 0, 5);
        for(int a = 0; a < 5; a++)
          dispense_qty[a] = 0;
//        moveServo();
        state = 1; 
        didUpdateView = false;
      }
      else if(lcd.currentWidget == 14)
      {
        //memcpy(dispense_qty, resistor_qty, 5);
        for(int a = 0; a < 5; a++)
          dispense_qty[a] = resistor_qty[a];
        state = 2;
        didUpdateView = false; 
      }
      else if(lcd.currentWidget == 27)
      {
        state = 3;
        didUpdateView = false;
      }
    }
    break;
  case 1:
    {
      lcd.wstack(0);
      int a = lcd.currentWidget - 2;

      if(a == 10)
      {
        moveServo();
          for(int a = 0; a < 5; a++)
          {
            if(dispense_qty[a]){
               take_qty(a, dispense_qty[a]);
                 store_qty();  
            }
          }
//          state = 0;
//          didUpdateView = false;
      }
      else if(a == 11)
      {
        state = 0;
        didUpdateView = false; 
      }
      else if(a >=0 && a <=9)
      {

        int resistor = a/2;
        boolean add = (a%2 == 0)? false: true;
        if(add)
        {
          if(resistor_qty[resistor] > dispense_qty[resistor]){
            dispense_qty[resistor] += 1;
            didUpdateView = false;
          }

        }
        else
        {
          if(dispense_qty[resistor] > 0){
            dispense_qty[resistor] -= 1;
            didUpdateView = false;  
          }
        }
      }

    }
    break;
  case 2:
    {
      lcd.wstack(0);
      int a = lcd.currentWidget - 15;

      if(a == 10)
      {
        state = 0;
        didUpdateView = false; 

      }
      else if(a == 11)
      {
        for(int b = 0; b < 5; b++)
        {
          set_qty(b, dispense_qty[b]); 
        }
        store_qty();
//send_empty(1);

        state = 0;
        didUpdateView = false;
      }
      else if(a >=0 && a <=9)
      {

        int resistor = a/2;
        boolean add = (a%2 == 0)? false: true;
        if(add)
        {
          dispense_qty[resistor] += 1;
          didUpdateView = false;
        }
        else
        {
          if(dispense_qty[resistor] > 0){
            dispense_qty[resistor] -= 1;
            didUpdateView = false;  
          }
        }
      }
    }
    break;
  case 3:
    {
      lcd.wstack(0);
      if(lcd.currentWidget == 28)
      {
        state = 0;
        didUpdateView = false; 
      }
    }
    break;  
  } 

}


void moveServo()
{
  digitalWrite(sol1,1);
  digitalWrite(sol2,0);
  delay(1000);
  digitalWrite(sol1,0);
  digitalWrite(sol2,1);

  
}

void openServo()
{
 digitalWrite(sol1,1);
  digitalWrite(sol2,0);
  delay(1000);
  digitalWrite(sol1,0);
  digitalWrite(sol2,1);
  delay(1000);
}


void get_qty()
{
  for (int i = 0; i < 5; i++)
  {
    resistor_qty[i] = EEPROM.read(i);
  }
  //    Serial.println(F("Got values."));
  //    Serial.println(F("\r\n"));

}

void store_qty()
{
  for (int i = 0; i < 5; i++)
  {
    EEPROM.write(i, resistor_qty[i]);
  }

  //    Serial.println(F("Saved values."));
  //    Serial.println(F("\r\n"));
//      check_empty();
//  print_r();
}

void add_qty(int rnum, int amount)
{
  resistor_qty[rnum] += amount;
  //    Serial.print("Added ");
  //    Serial.print(amount);
  //    Serial.print(" of ");
  //    Serial.println(rnum);
  //    Serial.println(F("\r\n"));
  store_qty();
}

void set_qty(int rnum, int amount)
{
  resistor_qty[rnum] = amount;
  //    Serial.println(F("Set values"));
  //    Serial.println(F("\r\n"));
}

int take_qty(int rnum, int amount)
{
  if(resistor_qty[rnum] < amount)
  {
    resistor_qty[rnum] = 0;
//    Serial.println("Not enough resistors to remove");
    return -1;
  }

  resistor_qty[rnum] -= amount;
  //    Serial.print("Took ");
  //    Serial.print(amount);
  //    Serial.print(" of ");
  //    Serial.println(rnum);
  //    Serial.println(F("\r\n"));
  return 1;
}

int check_num(int r[5])
{
  int tempr[5];
  memcpy(tempr, resistor_qty, 5);
  for(int i = 0; ; i++)
  {
    for(int j = 0; j < 5; j++)
    {
      if (tempr[j] -= r[j] <= 0)
        return i;
    }
  }
}


void dispense(int r[5], int qty)
{
  for(int i = 0; i > qty; i++)
  {
    resistor_qty[i] -= r[i];
  }
  check_empty();
  store_qty();
}

void check_empty()
{
  for(int i = 0; i < 5; i++)
  {
    if (resistor_qty[i] <= 0)
      send_empty(i);
  }
}

void send_empty(int rnum)
{
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("GET "));
    www.fastrprint(WEBPAGE);
    www.print(rnum);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: "));
    www.fastrprint(WEBSITE);
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } 
  else {
//    Serial.println(F("Connection failed"));
  lcd.cls(WHITE, BLACK);
    return;
  }

//  Serial.println(F("-------------------------------------"));

  /* Read data until either the connection is closed, or the idle timeout is reached. */
//  unsigned long lastRead = millis();
//  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
//    while (www.available()) {
//      char c = www.read();
////      Serial.print(c);
//      lastRead = millis();
//    }
//  }
  www.close();
//  Serial.println(F("-------------------------------------"));


}

void print_r()
{
  //    for(int i = 0; i < 5; i++)
  //    {
  //        Serial.print("Resistor "); Serial.print(i); Serial.print(": ");
  //        Serial.println(resistor_qty[i]);
  //    }
  //    Serial.print(F("\r\n"));
}

void init_wifi()
{
  if (!cc3000.begin())
  {
//    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
//    Serial.println(F("Failed!"));
    while(1);
  }

//  Serial.println(F("Connected!"));

  /* Wait for DHCP to complete */
//  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }

  ip = 0;
  // Try looking up the website's IP address
//  Serial.print(WEBSITE); 
//  Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
//      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }
  cc3000.printIPdotsRev(ip);
//  Serial.println(F("\r\n"));

}





