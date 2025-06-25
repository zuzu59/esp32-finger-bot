// Version hyper simpliste pour allumer ou éteindre la led via une page web

// Utilisation: http://adrs_ip

//
// ATTENTION, ce code a été testé sur un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION        "zf250625.0948"
#define zHOST           "simplet-led"              // ATTENTION, tout en minuscule

// General
const int ledPin = 8;             // the number of the LED pin
const int buttonPin = 9;          // the number of the pushbutton pin

#include "secrets.h"

#include <WiFi.h>
#include <WebServer.h>

// Remplacez par vos informations d'identification WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);

// Variable pour stocker l'état de la LED
bool ledState = LOW;

void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'></head><body>";
  html += "<h1>Contrôle de la LED ESP32-C3</h1>";
  html += "<p>Version: " + String(zVERSION) + "</p>";
  html += "<p>État actuel de la LED : " + String(ledState ? "Éteinte" : "Allumée") + "</p>";
  html += "<form method='get' action='/toggle'>";
  html += "<button type='submit'>Basculer LED</button>";
  html += "</form>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleToggle() {
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(19200);
  Serial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
  delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
  Serial.println("\n\n\n\n**************************************\nCa commence !"); Serial.println(zHOST ", " zVERSION);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  // Connexion au réseau WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au WiFi...");
  }

  Serial.println("Connecté au WiFi");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  // Configuration des routes du serveur
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  // Démarrage du serveur
  server.begin();
}

void loop() {
  server.handleClient();
}
