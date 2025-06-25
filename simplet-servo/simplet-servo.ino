// Version hyper simpliste pour bouger de A à B un servo moteur via une page web

// Utilisation: http://adrs_ip

// Il faut installer sur Arduino IDE la lib ESP32Servo de Kevin Harrington et John K. Bennett

//
// ATTENTION, ce code a été testé sur un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION        "zf250625.1023"
#define zHOST           "simplet-servo"              // ATTENTION, tout en minuscule


// General
const int ledPin = 8;             // the number of the LED pin
const int buttonPin = 9;          // the number of the pushbutton pin

#include "secrets.h"

#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

// Remplacez par vos informations WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);
Servo myservo;

int servoPin = 0; // Broche où le servo est connecté
int currentPos = 0; // Variable pour stocker la position actuelle du servo

void setup() {
  Serial.begin(19200);
  Serial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
  delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
  Serial.println("\n\n\n\n**************************************\nCa commence !"); Serial.println(zHOST ", " zVERSION);

  myservo.attach(servoPin);

  // Connexion au réseau WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au WiFi...");
  }
  Serial.println("Connecté au WiFi");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  // Configuration des routes du serveur web
  server.on("/", handleRoot);
  server.on("/set20", handleSet20);
  server.on("/set80", handleSet80);

  server.begin();
  Serial.println("Serveur web démarré");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<html><head><title>Contrôle Servo</title>";
  html += "<meta charset='UTF-8'>"; // Spécifie le jeu de caractères UTF-8
  html += "</head><body>";
  html += "<h1>Contrôle du Servo Moteur</h1>";
  html += "<p>Version: " + String(zVERSION) + "</p>";
  html += "<p>Position actuelle du servo: " + String(currentPos) + " degrés</p>";
  html += "<form action='/set20'><input type='submit' value='20 degrés'></form>";
  html += "<form action='/set80'><input type='submit' value='80 degrés'></form>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleSet20() {
  currentPos = 20;
  myservo.write(currentPos);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSet80() {
  currentPos = 80;
  myservo.write(currentPos);
  server.sendHeader("Location", "/");
  server.send(303);
}
