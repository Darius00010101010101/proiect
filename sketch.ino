#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

const char* topic_comanda = "proiect_ps/garaj/comanda";
const char* topic_status = "proiect_ps/garaj/stare";
const char* topic_distanta = "proiect_ps/garaj/distanta";

WiFiClient espClient;
PubSubClient client(espClient);
Servo usaGaraj;

#define SERVO_PIN 18
#define TRIG_PIN 5
#define ECHO_PIN 17
#define LED_OPEN 12
#define LED_CLOSED 14

long durata;
int distanta;
bool esteDeschis = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectare la ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectat!");
}

void deschideUsa() {
  if (!esteDeschis) {
    Serial.println("Actionare: Deschidere usa...");
    for (int pos = 0; pos <= 90; pos += 1) { 
      usaGaraj.write(pos);
      delay(15);
    }
    digitalWrite(LED_CLOSED, LOW);
    digitalWrite(LED_OPEN, HIGH);
    esteDeschis = true;
    client.publish(topic_status, "DESCHIS");
  }
}

void inchideUsa() {
  if (esteDeschis) {
    Serial.println("Actionare: Inchidere usa...");
    for (int pos = 90; pos >= 0; pos -= 1) { 
      usaGaraj.write(pos);
      delay(15);
    }
    digitalWrite(LED_OPEN, LOW);
    digitalWrite(LED_CLOSED, HIGH);
    esteDeschis = false;
    client.publish(topic_status, "INCHIS");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mesaj = "";
  for (int i = 0; i < length; i++) {
    mesaj += (char)payload[i];
  }
  Serial.print("Comanda primita: ");
  Serial.println(mesaj);

  if (String(topic) == topic_comanda) {
    if (mesaj == "1") deschideUsa();
    else if (mesaj == "0") inchideUsa();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectare MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectat!");
      client.subscribe(topic_comanda);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  usaGaraj.attach(SERVO_PIN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_CLOSED, OUTPUT);
  
  usaGaraj.write(0);
  digitalWrite(LED_CLOSED, HIGH);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Citire senzor ultrasonic
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  durata = pulseIn(ECHO_PIN, HIGH);
  distanta = durata * 0.034 / 2;
  
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    char distString[8];
    dtostrf(distanta, 1, 2, distString);
    client.publish(topic_distanta, distString);
  }
}
