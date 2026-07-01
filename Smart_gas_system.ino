/***************************************************
 * IoT Smart Gas, Flame & Sound Detection System
 * Wemos D1 Mini (ESP8266)
 * Blynk + Telegram + LCD
 ***************************************************/

//--------------- BLYNK ------------------
#define BLYNK_TEMPLATE_ID "TMPL3VKXdactq"
#define BLYNK_TEMPLATE_NAME "Gas Detection System"
#define BLYNK_AUTH_TOKEN "NzkqDECo81l2103OtfSn4-vawy8k17Q_"

#define BLYNK_PRINT Serial

//--------------- LIBRARIES ----------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

//--------------- WIFI --------------------
char ssid[] = "iPhone";
char pass[] = "Anesha24";

//--------------- TELEGRAM ----------------
#define BOT_TOKEN "8759467537:AAEqAZ_5EtsO-IkUZDPHrMr2Gc_fe_hJBAE"
#define CHAT_ID "7949478408"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

//--------------- PIN CONFIGURATION ----------------

#define MQ2_PIN A0

#define FLAME_PIN D4
#define SOUND_PIN D7

#define BUZZER_PIN D5

#define LED_RED D3
#define LED_GREEN D0

LiquidCrystal_I2C lcd(0x27,16,2);

BlynkTimer timer;

//--------------- VARIABLES ----------------

int GAS_THRESHOLD = 500;

bool systemOn=true;
bool buzzerRemote=false;

bool notified=false;

//--------------- BLYNK PINS ----------------

#define VPIN_GAS V0
#define VPIN_FLAME V1
#define VPIN_BUZZER V2
#define VPIN_SYSTEM V3

//--------------- BLYNK BUTTONS ----------------

BLYNK_WRITE(VPIN_BUZZER)
{
  buzzerRemote=param.asInt();
}

BLYNK_WRITE(VPIN_SYSTEM)
{
  systemOn=param.asInt();
}

//--------------- SENSOR FUNCTION ----------------

void readSensors()
{

int gasValue=analogRead(MQ2_PIN);

bool flameDetected=(digitalRead(FLAME_PIN)==LOW);

bool soundDetected=(digitalRead(SOUND_PIN)==HIGH);

bool danger=(gasValue>GAS_THRESHOLD || flameDetected || soundDetected);

// Send values to Blynk

Blynk.virtualWrite(VPIN_GAS,gasValue);
Blynk.virtualWrite(VPIN_FLAME,flameDetected);

//---------------- SYSTEM OFF ----------------

if(!systemOn)
{

lcd.clear();

lcd.setCursor(0,0);
lcd.print("SYSTEM OFF");

digitalWrite(LED_RED,LOW);
digitalWrite(LED_GREEN,LOW);

digitalWrite(BUZZER_PIN,LOW);

return;

}

//---------------- LCD LINE 1 ----------------

lcd.setCursor(0,0);

lcd.print("G:");
lcd.print(gasValue);

lcd.print(" F:");
lcd.print(flameDetected);

lcd.print(" S:");
lcd.print(soundDetected);

lcd.print(" ");

//---------------- ALERT MESSAGE ----------------

String msg="";

if(gasValue>GAS_THRESHOLD)
msg+="Gas Detected\n";

if(flameDetected)
msg+="Flame Detected\n";

if(soundDetected)
msg+="Sound Detected\n";

//---------------- DANGER DETECTED ----------------

if(danger)
{

lcd.setCursor(0,1);
lcd.print("                ");
lcd.setCursor(0,1);

if(gasValue>GAS_THRESHOLD && !flameDetected && !soundDetected)
lcd.print("Gas Detected");

else if(flameDetected && gasValue<=GAS_THRESHOLD && !soundDetected)
lcd.print("Flame Detected");

else if(soundDetected && gasValue<=GAS_THRESHOLD && !flameDetected)
lcd.print("Sound Detected");

else
lcd.print("Multiple Alert");

digitalWrite(LED_RED,HIGH);
digitalWrite(LED_GREEN,LOW);

// Buzzer
if(danger || buzzerRemote)
digitalWrite(BUZZER_PIN,HIGH);
else
digitalWrite(BUZZER_PIN,LOW);

// Telegram + Blynk notification
if(!notified)
{

Blynk.logEvent("danger_alert",msg);

bot.sendMessage(CHAT_ID,
"⚠ ALERT\n\n"+msg,
"");

notified=true;

}

}
else
{

lcd.setCursor(0,1);
lcd.print("SAFE            ");

digitalWrite(LED_RED,LOW);
digitalWrite(LED_GREEN,HIGH);

if(buzzerRemote)
digitalWrite(BUZZER_PIN,HIGH);
else
digitalWrite(BUZZER_PIN,LOW);

notified=false;

}

}

//---------------- SETUP ----------------

void setup()
{

Serial.begin(115200);

pinMode(MQ2_PIN,INPUT);

pinMode(FLAME_PIN,INPUT);

pinMode(SOUND_PIN,INPUT);

pinMode(BUZZER_PIN,OUTPUT);

pinMode(LED_RED,OUTPUT);

pinMode(LED_GREEN,OUTPUT);

digitalWrite(BUZZER_PIN,LOW);

digitalWrite(LED_RED,LOW);

digitalWrite(LED_GREEN,LOW);

// LCD

Wire.begin(D2,D1);

lcd.init();

lcd.backlight();

lcd.clear();

lcd.setCursor(0,0);

lcd.print("Starting...");

// Telegram SSL

client.setInsecure();

// WiFi + Blynk

Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);

// Startup Telegram

bot.sendMessage(CHAT_ID,
"🚀 Smart Gas Detection System Started Successfully.",
"");

// Timer

timer.setInterval(500L,readSensors);

}

//---------------- LOOP ----------------

void loop()
{

Blynk.run();

timer.run();

}
