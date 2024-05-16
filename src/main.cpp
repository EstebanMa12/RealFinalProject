#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define LEDPin 2
#define PIRPin 18
#define dipSwitch 19

#define SSID "esteban"
#define PASS "david19mase"

int pirState = LOW; // de inicio no hay movimiento
int val = 0;
int dipState = 0;
int movements = 0;
bool ledIsOn = false;

String ClientId;

const char *MQTT_Server = {"broker.hivemq.com"};
const int MQTT_Port = 1883;
String topic = "esteban/PIR/";

WiFiClient wifiClient;
PubSubClient MQTTClient(wifiClient);

void InitWiFi();
void connectToMQTT();

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
  connectToMQTT();
  int val = digitalRead(PIRPin);
  bool dipState = digitalRead(dipSwitch) == HIGH;

  if (val == HIGH && !ledIsOn)
  {                             // Si se detecta movimiento y el LED no está encendido
    digitalWrite(LEDPin, HIGH); // Enciende el LED
    movements++;                // Incrementa los movimientos
    ledIsOn = true;             // Actualiza el estado del LED como encendido
    if (pirState == LOW)
    { // Si previamente estaba apagado
      Serial.println("Sensor activado");
      pirState = HIGH;
    }
  }
  else if (!val && ledIsOn)
  {                            // Si no se detecta movimiento pero el LED estaba encendido
    digitalWrite(LEDPin, LOW); // Apaga el LED
    ledIsOn = false;           // Actualiza el estado del LED como apagado
    if (pirState == HIGH)
    { // Si previamente estaba encendido
      Serial.println("Sensor parado");
      pirState = LOW;
    }
  }

  StaticJsonDocument<200> doc;
  JsonObject object = doc.to<JsonObject>();
  object["movimientos"] = movements;
  object["estado"] = dipState == HIGH ? "ON" : "OFF";

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  String message = "El sensor ha detectado " + String(movements) + " movimientos, y el estado del boton es: ";
  message += dipState == HIGH ? "ON" : "OFF";

  if (MQTTClient.publish(topic.c_str(), buffer, n))
  {
    Serial.println(message);
  }

  delay(500);
}

void connectToMQTT()
{
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