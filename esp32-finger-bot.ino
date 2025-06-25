// Petit robot pour bouger de A à B un servo moteur via une page web
//

#define zVERSION        "zf250625.1132"
#define zHOST           "finger-bot1"              // ATTENTION, tout en minuscule
#define zDSLEEP         0                       // 0 ou 1 !


#define TIME_TO_SLEEP   300                 // dSleep en secondes 
int zDelay1Interval =   5000;              // Délais en mili secondes pour la boucle loop

/*

Utilisation:

Simplement avec un browser web aller sur http://adrs_ip

ATTENTION, ce code a été testé sur un esp32-c3 super mini. Pas testé sur les autres boards !


Installation:

Pour les esp32-c3 super mini, il faut:
 * choisir comme board ESP32C3 Dev Module
 * enabled USB CDC On Boot si on veut que la console serial fonctionne !
 * changer le schéma de la partition à Minimal SPIFFS (1.9MB APP with OTA/190kB SPIFFS)

Pour le WiFiManager, il faut installer cette lib depuis le lib manager sur Arduino:
https://github.com/tzapu/WiFiManager

Pour le senseur DS18B20 il faut installer ces lib: 
https://github.com/PaulStoffregen/OneWire                             OneWire
https://github.com/milesburton/Arduino-Temperature-Control-Library    DallasTemperature

Pour MQTT, il faut installer la lib (home-assistant-integration):
https://github.com/dawidchyrzynski/arduino-home-assistant

Pour JSON, il faut installer cette lib:
https://github.com/bblanchon/ArduinoJson

Pour la lib du servo moteur Il faut installer ESP32Servo de Kevin Harrington et John K. Bennett

Sources:
https://www.reddit.com/r/esp32/comments/1crwakg/built_in_temperature_sensor_on_esp32c3_red_as/?rdt=63263
https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561
https://www.aliexpress.com/item/1005006005040320.html
https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino
https://dronebotworkshop.com/wifimanager/
https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/
https://github.com/dawidchyrzynski/arduino-home-assistant/blob/main/examples/sensor-integer/sensor-integer.ino
https://chat.mistral.ai/    pour toute la partie API REST et wifiAuto ᕗ
*/




// #define DEBUG true
// #undef DEBUG



// General
const int ledPin = 8;             // the number of the LED pin
const int buttonPin = 9;          // the number of the pushbutton pin
#include "secrets.h"


// Sonar Pulse
#include "zSonarpulse.h"


// WIFI
#define lowTxPower  true   // diminution de la puissance à cause de la réflexion de l'antenne sur le HTU21D directement soudé sur le esp32-c3 super mini zf240725.1800
#include "zWifi.h"


// OTA WEB server
#include "otaWebServer.h"


// Servo WEB server
#include "servoWebServer.h"

#include <ESP32Servo.h>
Servo myservo;
int servoPin = 0; // Broche où le servo est connecté
int currentPos = 30; // Variable pour stocker la position actuelle du servo
myservo.attach(servoPin);



// MQTT
#include "zMqtt.h"


// Temperature sensor
#include "zTemperature.h"

#if zDSLEEP == 1
  // Deep Sleep
  #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
  // #define TIME_TO_SLEEP  300      /* Time ESP32 will go to sleep (in seconds) */
  RTC_DATA_ATTR int bootCount = 0;
#endif


void setup() {
  // Il faut lire la température tout de suite au début avant que le MCU ne puisse chauffer !
  initDS18B20Sensor();
  delay(200);
  readSensor();

  // Pulse deux fois pour dire que l'on démarre
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); delay(zSonarPulseOn); digitalWrite(ledPin, HIGH); delay(zSonarPulseOff);
  digitalWrite(ledPin, LOW); delay(zSonarPulseOn); digitalWrite(ledPin, HIGH); delay(zSonarPulseOff);
  delay(zSonarPulseWait);

  // Start serial console
  Serial.begin(19200);
  Serial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
  delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
  Serial.println("\n\n\n\n**************************************\nCa commence !"); Serial.println(zHOST ", " zVERSION);

  #if zDSLEEP == 1
    //Increment boot number and print it every reboot
    ++bootCount;
    sensorValue4 = bootCount;
    Serial.println("Boot number: " + String(bootCount));
    // Configuration du dsleep
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  #endif

  // Start WIFI
  zStartWifi();
  sensorValue3 = WiFi.RSSI();

  // Start OTA server
  otaWebServer();

  // Start servoWebServer
  servoWebServer();

  // Connexion au MQTT
  Serial.println("\n\nConnect MQTT !\n");
  ConnectMQTT();

  // go go go
  Serial.println("\nC'est parti !\n");

  // Envoie toute la sauce !
  zEnvoieTouteLaSauce();
  Serial.println("\nC'est envoyé !\n");

  #if zDSLEEP == 1
    // Partie dsleep. On va dormir !
    Serial.println("Going to sleep now");
    delay(200);
    Serial.flush(); 
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  #endif

}

void loop() {
  // Envoie toute la sauce !
  //zEnvoieTouteLaSauce();
  handleClient();
  // Délais non bloquant pour le sonarpulse et l'OTA
  zDelay1(zDelay1Interval);
}


// Envoie toute la sauce !
void zEnvoieTouteLaSauce(){

  // Lit les températures
  readSensor();

  // Envoie les mesures au MQTT
  sendSensorMqtt();

  // Graphe sur l'Arduino IDE les courbes des mesures
  Serial.print("sensor1:");
  Serial.print(sensorValue1);
  Serial.print(",tempInternal1:");
  Serial.print(tempInternal1);
  Serial.print(",tempInternal2:");
  Serial.print(tempInternal2);

  // Serial.print(",sensor2:");
  // Serial.print(sensorValue2);
  // Serial.print(",sensor3:");
  // Serial.print(sensorValue3);
  // Serial.print(",sensor4:");
  // Serial.print(sensorValue4);
  // Serial.print(",sensor5:");
  // Serial.print(sensorValue5);
  Serial.println("");
}


// Délais non bloquant pour le sonarpulse et l'OTA
void zDelay1(long zDelayMili){
  long zDelay1NextMillis = zDelayMili + millis(); 
  while(millis() < zDelay1NextMillis ){
    // OTA loop
    server.handleClient();
    // Un petit coup sonar pulse sur la LED pour dire que tout fonctionne bien
    sonarPulse();
  }
}

