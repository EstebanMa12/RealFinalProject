#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define LEDPin 2
#define PIRPin 18
#define dipSwitch 19

#define SSID "esteban"
#define PASS "david19mase"

bool pirState = LOW; // de inicio no hay movimiento
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 20000;

int val = 0;
int dipState = 0;

String ClientId;

const char *MQTT_Server = {"broker.hivemq.com"};
const int MQTT_Port = 1883;
String topic = "esteban/PIR/";

WiFiClient wifiClient;
PubSubClient MQTTClient(wifiClient);

void InitWiFi();
void connectToMQTT();
void publishSensorState(bool motionDetected, bool dipState);
void handlePIRSensor(int pirVal) ;
void handlePublishing(bool dipState);

void setup()
{
  Serial.begin(9600);
  pinMode(LEDPin, OUTPUT);
  pinMode(PIRPin, INPUT_PULLUP);
  pinMode(dipSwitch, INPUT_PULLDOWN);
  InitWiFi();

  MQTTClient.setServer(MQTT_Server, MQTT_Port);
}

void loop()
{
  if (!MQTTClient.connected()) {
    connectToMQTT();
  }
  MQTTClient.loop();
  int val = digitalRead(PIRPin);
  bool dipState = digitalRead(dipSwitch) == HIGH;

  handlePIRSensor(val);
  handlePublishing(dipState);
}

void handlePIRSensor(int pirVal) {
  if (pirVal == HIGH) {
    digitalWrite(LEDPin, HIGH);
    if (pirState == LOW) {
      Serial.println("Sensor activado");
      pirState = HIGH;
    }
  } else {
    digitalWrite(LEDPin, LOW);
    if (pirState == HIGH) {
      Serial.println("Sensor parado");
      pirState = LOW;
    }
  }
}

void handlePublishing(bool dipState) {
  static unsigned long lastPublishTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastPublishTime >= publishInterval) {
    publishSensorState(pirState, dipState);
    lastPublishTime = currentTime;
  }
}

void publishSensorState(bool motionDetected, bool dipState)
{
  StaticJsonDocument<200> doc;
  JsonObject object = doc.to<JsonObject>();
  object["movementState"] = motionDetected;
  object["buttonState"] = dipState;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  if (MQTTClient.publish(topic.c_str(), buffer, n))
  {
    String message = "El sensor está " + String(motionDetected ? "activo" : "inactivo") + ", y el estado del botón es: " + (dipState ? "ON" : "OFF");
    Serial.println(message);
  }
}

void connectToMQTT()
{
  Serial.print("Attempting MQTT connection...");
  if (!MQTTClient.connected())
  {
    ClientId = String(random(1000));
    if (MQTTClient.connect(ClientId.c_str()))
    {
      Serial.println("Conectado al servidor MQTT");
    }
    else
    {
      Serial.println("Error al conectar al servidor MQTT");
    }
  }
}

void InitWiFi()
{
  WiFi.mode(WIFI_STA);

  WiFi.begin(SSID, PASS);
  Serial.print("Conectando a la red ");
  Serial.print(SSID);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("Conectado");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  else
  {
    Serial.println("Error al conectar");
  }

  Serial.println("Conexión exitosa");
}