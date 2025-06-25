// Petit robot pour bouger de A à B un servo moteur via une page web
// Et cerise sur le gâteau envoie la température du esp32-c3 ainsi que le compteur de loop à un swerveur mqtt !
//

#define zVERSION        "zf250625.1500"
#define zHOST           "finger-bot1"              // ATTENTION, tout en minuscule
#define zDSLEEP         0                       // toujours à 0, pour comptabilité avec ma caisse à outils !

int zDelay1Interval =   15000;              // Délais en mili secondes pour la boucle loop

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
#include <WebServer.h>
WebServer servoWebServer(80);

#include <ESP32Servo.h>
Servo myservo;
int servoPin = 0; // Broche où le servo est connecté
int onPos = 70; // Variable pour stocker la position actuelle du servo
int offPos = 30; // Variable pour stocker la position actuelle du servo
int currentPos = offPos; // Variable pour stocker la position actuelle du servo


// MQTT
#include "zMqtt.h"


// Temperature sensor
#include "zTemperature.h"
int bootCount = 0;



void setup() {
  // Il faut lire la température tout de suite au début avant que le MCU ne puisse chauffer !
  // initDS18B20Sensor();
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

  // Start WIFI
  zStartWifi();
  sensorValue3 = WiFi.RSSI();

  // Start OTA server
  otaWebServer();

  // Start servo
  myservo.attach(servoPin);
  myservo.write(currentPos);
  
  // Configuration des routes du serveur web
  servoWebServer.on("/", handleRoot);
  servoWebServer.on("/setOFF", handleSetOFF);
  servoWebServer.on("/setON", handleSetON);
  servoWebServer.begin();
  Serial.println("Serveur web démarré");

  // Connexion au MQTT
  Serial.println("\n\nConnect MQTT !\n");
  ConnectMQTT();

  // go go go
  Serial.println("\nC'est parti !\n");

  // Envoie toute la sauce !
  zEnvoieTouteLaSauce();
  Serial.println("\nC'est envoyé !\n");

}

void loop() {

  //Increment boot number
  ++bootCount;
  sensorValue4 = bootCount;

  // Envoie toute la sauce !
  zEnvoieTouteLaSauce();
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


// Délais non bloquant pour le sonarpulse et l'OTA et le WEB server pour le servo
void zDelay1(long zDelayMili){
  long zDelay1NextMillis = zDelayMili + millis(); 
  while(millis() < zDelay1NextMillis ){
    // OTA loop
    server.handleClient();

    // Servo WEB server loop
    servoWebServer.handleClient();

    // Un petit coup sonar pulse sur la LED pour dire que tout fonctionne bien
    sonarPulse();
  }
}

void handleRoot() {
  String html = "<html><head><title>Contrôle Servo</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; // Balise meta viewport
  html += "<style>";
  html += "body { font-size: 20px; text-align: center; }"; // Style CSS pour le corps du texte
  html += "button { padding: 15px; font-size: 18px; margin: 10px; }"; // Style pour les boutons
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Contrôle du Servo Moteur</h1>";
  html += "<p>Version: " + String(zHOST) + " " + String(zVERSION) + "</p>";
  html += "<p>Position actuelle du servo (~30=OFF, ~70=ON): " + String(currentPos) + " degrés</p>";
  html += "<form action='/setOFF'><input type='submit' value='OFF' style='padding: 15px; font-size: 18px;'></form>";
  html += "<form action='/setON'><input type='submit' value='ON' style='padding: 15px; font-size: 18px;'></form>";
  html += "</body></html>";

  servoWebServer.send(200, "text/html", html);
}

void handleSetOFF() {
  currentPos = offPos;
  myservo.write(currentPos);
  servoWebServer.sendHeader("Location", "/");
  servoWebServer.send(303);
}

void handleSetON() {
  currentPos = onPos;
  myservo.write(currentPos);
  servoWebServer.sendHeader("Location", "/");
  servoWebServer.send(303);
}

