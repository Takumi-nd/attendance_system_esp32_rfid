#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>

// put define
#define RST_PIN 0
#define SS_PIN 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define BUTTON_PIN 33
#define BACK_BUTTON_PIN 25
#define BUZZER 4

// global variable
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const char *ssid = "NAME_WIFI";
const char *password = "PASS_WIFI";
const char *postServer = "https://embwebapp.000webhostapp.com/post-esp-data.php";
const char *updateServer = "https://embwebapp.000webhostapp.com/update-checkin.php";
String getServer = "https://embwebapp.000webhostapp.com/get-id.php";
String apiKeyValue = "tPmAT5Ab3j7F9";

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
//  store cardID from mifare 1K card
byte cardID[4];
// convert UID from mifare 1K card to String
String cardIdString;
// basic Info
String name;
String phone;
// if read ID success readIdSuccess will be true
bool readIdSuccess = false;
// if Id exist on server isIdExist will be true
bool isIdExist = false;
bool httpRequestSuccess = false;

const int shortPressTime = 500;
const int longPressTime = 1000;
int lastState = LOW;
int currentState;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;
// variables below use for change the screen
uint8_t change = 0;
uint8_t prevChange = 0;
bool mainScreen = true;

// put function declarations here:
// write new info and store as a String 
void newInfo();
// convert byte to Hex and store as a String
String byteToHex(byte value);
// read UID from mifare 1K card
void readID();

// main program 
void rfidProgram();

// this function use for detect press
  // hold the BUTTON_PIN about 1s to select function
  // press the BUTTON_PIN to change between functions
  // press BACK_BUTTON_PIN to return to the main screen
void detectPress();
void refreshMainScreen();
void refreshSubScreen();
void header();
void mainLayer();
// Interrupt generate each time the BACK_BUTTON_PIN is pressed
void IRAM_ATTR isr();

// this function INSERT Data to server
void insertData();
// this function will send the id from mifare 1K card and find it in the database.
void checkIdExist();
// this function will send the id from mifare 1K card and find it in the database, if id exist 
// checkin value in database will increase by 1.
void updateCheckin();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  /* io init */
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACK_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BACK_BUTTON_PIN, isr, FALLING);

  /* ssd1306 init */
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();

  /* mfrc522 init */
  SPI.begin();
  mfrc522.PCD_Init();
  /* all keys set to FFFFFFFFFFFFh at chip delivery from factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  /* wifi init */
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  // put your main code here, to run repeatedly:
  rfidProgram();
}

void newInfo()
{
  // input name
  Serial.println("--------------------");
  Serial.println("Type name");
  while (Serial.available() == 0)
  {
  }
  name = Serial.readStringUntil('\n');
  Serial.println(name);

  // input phone
  Serial.println("--------------------");
  Serial.println("Type phone");
  while (Serial.available() == 0)
  {
  }
  phone = Serial.readStringUntil('\n');
  Serial.println(phone);
}

String byteToHex(byte value) {
  char hexChars[] = "0123456789ABCDEF";
  String hexString = "";

  hexString += hexChars[(value >> 4) & 0x0F];
  hexString += hexChars[value & 0x0F];

  return hexString;
}

void readID()
{
  cardIdString = "";

  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    cardID[i] = mfrc522.uid.uidByte[i];
    cardIdString += byteToHex(cardID[i]);
  }
  display.setCursor(0, 10);
  display.print("Card ID: ");
  display.setCursor(50, 10);
  display.print(cardIdString);
  readIdSuccess = true;

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void rfidProgram()
{
  detectPress();

  if (mainScreen)
  {
    mainLayer();
  }

  if (!mainScreen)
  {
    if (change == 1)
    {
      display.setCursor(0, 0);
      display.print("---Check-In---");
      readID();
      if (readIdSuccess)
      {
        checkIdExist();
        if (isIdExist && httpRequestSuccess)
        {
          updateCheckin();
        }
        readIdSuccess = false;
      }
    }

    if (change == 2)
    {
      display.setCursor(0, 0);
      display.print("---Use your PC!---");
      readID();
      if (readIdSuccess)
      {
        checkIdExist();
        if (isIdExist == false && httpRequestSuccess == true)
        {
          refreshSubScreen();
          newInfo();
          //displayInfo();
          insertData();
        }
        readIdSuccess = false;
      }
    }

    if (change == 3)
    {
      display.setCursor(0, 32);
      display.print("COMMING SOON!");
    }
    refreshSubScreen();
  }
}

void detectPress()
{
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) // button is pressed
  {
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  }
  else if (lastState == LOW && currentState == HIGH) // button is released
  {
    isPressing = false;
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration < shortPressTime)
    {
      // Serial.println("A short press is detected");
      change++;
    }
  }

  if (isPressing == true && isLongDetected == false)
  {
    long pressDuration = millis() - pressedTime;

    if (pressDuration > longPressTime)
    {
      Serial.println(pressDuration);
      isLongDetected = true;
      prevChange = change;
      mainScreen = false;
    }
  }

  // save the the last state
  lastState = currentState;
}

void refreshMainScreen()
{
  display.display();
  //delay(1500);
  display.clearDisplay();
}

void refreshSubScreen()
{
  display.display();
  delay(1500);
  display.clearDisplay();
}

void header()
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22, 0);
  display.print("Rfid");
  display.drawLine(0, 8, 128, 9, SSD1306_WHITE);
}

void mainLayer()
{
  if (change == 1)
  {
    header();
    display.setCursor(0, 20);
    display.print(">> Check-In");
    display.setCursor(0, 29);
    display.print("New card");
    display.setCursor(0, 38);
    display.print("Comming soon");
    display.setCursor(0, 47);
    refreshMainScreen();
  }

  if (change == 2)
  {
    header();
    display.setCursor(0, 20);
    display.print("Check-In");
    display.setCursor(0, 29);
    display.print(">> New card");
    display.setCursor(0, 38);
    display.print("Comming soon");
    display.setCursor(0, 47);
    refreshMainScreen();
  }

  if (change == 3)
  {
    header();
    display.setCursor(0, 20);
    display.print("Check-In");
    display.setCursor(0, 29);
    display.print("New card");
    display.setCursor(0, 38);
    display.print(">> Comming soon");
    display.setCursor(0, 47);
    refreshMainScreen();
  }

  if (change >= 4)
  {
    change = 1;
  }
}

void IRAM_ATTR isr()
{
  mainScreen = true;
  change = prevChange;
}

void insertData()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(postServer);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "api_key=" + apiKeyValue + "&cardid=" + cardIdString + "&name=" + name + "&phone=" + phone;

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0)
    {
      display.setCursor(0, 32);
      display.print("Insert successfully!");
    }
    else
    {
      display.setCursor(0, 32);
      display.print("Insert failed!");
    }

    http.end();
  }
  else
  {
    display.setCursor(0, 32);
    display.print("Wifi Disconnected");
  }
}

void checkIdExist()
{
  HTTPClient http;

  String getUrl = getServer + "?cardid=" + cardIdString;
  http.begin(getUrl.c_str());

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.println(httpResponseCode);
    if (httpResponseCode == HTTP_CODE_OK)
    {
      isIdExist = true;
      httpRequestSuccess = true;
      display.setCursor(0, 22);
      display.print("Cardid found!");
    }
    else
    {
      isIdExist = false;
      httpRequestSuccess = true;
      display.setCursor(0, 22);
      display.print("Cardid not found!");
    }
  }
  else
  {
    httpRequestSuccess = false;
    display.setCursor(0, 22);
    display.print("Error on HTTP request");
  }
  http.end();
}

void updateCheckin()
{
  HTTPClient http;
  http.begin(updateServer);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String httpRequestData = "api_key=" + apiKeyValue + "&cardid=" + cardIdString;
  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0)
  {
    display.setCursor(0, 32);
    display.print("Checkin successfully!");
    display.setCursor(0, 52);
    display.print("-->GET IN");
  }
  else
  {
    display.setCursor(0, 32);
    display.print("Checkin failed!");
  }

  http.end();
}
