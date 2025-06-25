// Petit serveur WEB simpliste pour bouger de A à B un servo moteur via une page web
//
// zf250625.1158
//
// Utilisation: http://adrs_ip
//
// Il faut installer sur Arduino IDE la lib ESP32Servo de Kevin Harrington et John K. Bennett
//
// ATTENTION, ce code a été testé sur un esp32-c3 super mini. Pas testé sur les autres boards !
//

// #include <ESP32Servo.h>
// Servo myservo;


#include <WebServer.h>

WebServer servoServer(80);

// int servoPin = 0; // Broche où le servo est connecté
// int currentPos = 30; // Variable pour stocker la position actuelle du servo

// myservo.attach(servoPin);


static void handleRoot() {
  String html = "<html><head><title>Contrôle Servo</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; // Balise meta viewport
  html += "<style>";
  html += "body { font-size: 20px; text-align: center; }"; // Style CSS pour le corps du texte
  html += "button { padding: 15px; font-size: 18px; margin: 10px; }"; // Style pour les boutons
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Contrôle du Servo Moteur</h1>";
  html += "<p>Version: " + String(zVERSION) + "</p>";
  html += "<p>Position actuelle du servo: " + String(currentPos) + " degrés</p>";
  html += "<form action='/set20'><input type='submit' value='20 degrés' style='padding: 15px; font-size: 18px;'></form>";
  html += "<form action='/set80'><input type='submit' value='80 degrés' style='padding: 15px; font-size: 18px;'></form>";
  html += "</body></html>";

  servoServer.send(200, "text/html", html);
}

static void handleSet20() {
  currentPos = 30;
  myservo.write(currentPos);
  servoServer.sendHeader("Location", "/");
  servoServer.send(303);
}

static void handleSet80() {
  currentPos = 70;
  myservo.write(currentPos);
  servoServer.sendHeader("Location", "/");
  servoServer.send(303);
}



static void servoWebServer() {
    // Configuration des routes du serveur web
  servoServer.on("/", handleRoot);
  servoServer.on("/set20", handleSet20);
  servoServer.on("/set80", handleSet80);

  servoServer.begin();
  Serial.println("Serveur web démarré");
}

static void handleClient() {
  servoServer.handleClient();
}


