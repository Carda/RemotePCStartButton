#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer webServ(80);
char buffer[200];
StaticJsonDocument<200> jDoc;

bool isConnected;
bool isStarted;

int ESP_PIN_RELAY = 12;
const String API_START_ARG = "on";
const String API_STOP_ARG = "off";
const String API_TRIG_ARG = "trig";
const String CONTENT_TYPE = "application/json";
const int HTTP_SUCCESS = 200;
const int HTTP_INTERNAL_ERROR = 500;
const int BLUELED = 4;
String newHostName = "ArdaTV_RemoteControl";

void BlinkBuiltinLed(int blinkTimeout)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(blinkTimeout);
  digitalWrite(LED_BUILTIN, LOW);
  delay(blinkTimeout);
}

void initiateBlinkTask(){
  
}

void SwitchBuiltinLed(bool isOn)
{
  digitalWrite(BLUELED, (isOn ? HIGH : LOW));
}

void SetRelayStatus()
{
  Serial.println("isStarted :");
  Serial.println((isStarted ? HIGH : LOW));
  digitalWrite(ESP_PIN_RELAY, (isStarted ? HIGH : LOW));
}

void ClearBuffer()
{
  memset(buffer, 0, sizeof(buffer));
}

void GetStatus()
{
  ClearBuffer();
  jDoc.clear();
  jDoc["power_stat"] = (isStarted ? 1 : 0);

  serializeJson(jDoc, buffer);
  webServ.send(200, CONTENT_TYPE, buffer);
}

void Start()
{
  ClearBuffer();
  if (webServ.hasArg("plain"))
  {
    String body = webServ.arg("plain");
    Serial.println("Deserialization is started for Start.!");
    DeserializationError error = deserializeJson(jDoc, body);
    Serial.println("Deserialization is finished for Start.!");
    if (error)
    {
      Serial.println("JSon deserialization Error..! : ");
      Serial.println(error.f_str());
    }

    if (jDoc["power"] == API_START_ARG)
    {
      Serial.println("Relay is opening.!");
      isStarted = true;
      digitalWrite(ESP_PIN_RELAY, LOW);
    }
    GetStatus();
  }
}

void Stop()
{
  ClearBuffer();
  if (webServ.hasArg("plain"))
  {
    String body = webServ.arg("plain");
    Serial.println("Deserialization is starting for Stop.!");
    DeserializationError error = deserializeJson(jDoc, body);
    Serial.println("Deserialization is finished for Stop.!");
    if (error)
    {
      Serial.println("JSon deserialization Error..! : ");
      Serial.println(error.f_str());
    }

    if (jDoc["power"] == API_STOP_ARG)
    {
      Serial.println("Relay is closing.!");
      isStarted = false;
      digitalWrite(ESP_PIN_RELAY, HIGH);
    }
    GetStatus();
  }
}

void Trigger()
{
  ClearBuffer();
  if (webServ.hasArg("plain"))
  {
    String body = webServ.arg("plain");
    Serial.println(body);
    Serial.println("Deserialization is starting for Stop.!");
    DeserializationError error = deserializeJson(jDoc, body);
    Serial.println("Deserialization is finished for Stop.!");
    if (error)
    {
      Serial.println("JSon deserialization Error..! : ");
      Serial.println(error.f_str());
    }

    String duration = jDoc["duration"];
    int dura = 0;
    if (!duration.isEmpty())
    {
      dura = duration.toInt();
    }

    if (jDoc["power"] == API_TRIG_ARG)
    {
      Serial.println("Relay is closing.!");
      isStarted = false;
      digitalWrite(ESP_PIN_RELAY, LOW);
      delay(dura);
      digitalWrite(ESP_PIN_RELAY, HIGH);
    }
    GetStatus();
  }
}

void ConfigurateRouting()
{
  webServ.on("/getStatus", GetStatus);
  webServ.on("/start", HTTP_POST, Start);
  webServ.on("/stop", HTTP_POST, Stop);
  webServ.on("/trigger", HTTP_POST, Trigger);
}

void setup()
{
  ClearBuffer();
  Serial.begin(115200);
  Serial.println("Setup is starting");
  pinMode(ESP_PIN_RELAY, OUTPUT);
  digitalWrite(ESP_PIN_RELAY, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);

  BlinkBuiltinLed(1500);

  WiFiManager wm;

  wm.setAPStaticIPConfig(IPAddress(192, 168, 33, 1), IPAddress(192, 168, 33, 1), IPAddress(255, 255, 255, 0));
  isConnected = wm.autoConnect("Remote Button for ArdaTV", "arda8585");

  WiFi.hostname(newHostName.c_str());

  while (WiFi.status() != WL_CONNECTED)
  {
    BlinkBuiltinLed(500);
  }

  SwitchBuiltinLed(isConnected = true);

  if (isConnected)
    Serial.println("Wifi Connection established");
  else
    Serial.println("Wifi connection failed.!");

  Serial.print("Mac Address : ");
  Serial.println(WiFi.macAddress());

  ConfigurateRouting();
  webServ.begin();
  Serial.println("Web Server Started!");
}

void loop()
{
  if (!isConnected || WiFi.status() != WL_CONNECTED)
  {
    delay(3000);
    ESP.restart();
  }

  webServ.handleClient();
}