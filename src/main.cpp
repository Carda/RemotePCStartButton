#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>

WebServer webServ(80);
char buffer[200];
StaticJsonDocument<200> jDoc;

bool isConnected;
bool isStarted;

int ESP_PIN_RELAY = 4;
const String API_START_ARG = "on";
const String API_STOP_ARG = "off";
const String CONTENT_TYPE = "application/json";
const int HTTP_SUCCESS = 200;
const int HTTP_INTERNAL_ERROR = 500;

IPAddress espServerip(192, 168, 0, 199);
IPAddress espGateway(192, 168, 0, 1);
IPAddress espSubnet(255, 255, 255, 0);

void BlinkBuiltinLed(int blinkTimeout)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(blinkTimeout);
  digitalWrite(LED_BUILTIN, LOW);
  delay(blinkTimeout);
}

void SwitchOnBuiltinLed(bool isOn)
{
  digitalWrite(LED_BUILTIN, (isOn ? HIGH : LOW));
}

void SetRelayStatus()
{
  Serial.println("isStarted :");
  Serial.println((isStarted ? HIGH : LOW));
  digitalWrite(ESP_PIN_RELAY, (isStarted ? HIGH : LOW));
}

void ClearBuffer(){
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
  try
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
  catch (const std::exception &e)
  {
    Serial.println("Hata .!");
    Serial.println(e.what());
    jDoc.clear();
    jDoc["error"] = e.what();
    serializeJson(jDoc, buffer);
    webServ.send(HTTP_INTERNAL_ERROR, CONTENT_TYPE, buffer);
  }
}

void Stop()
{
  try
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
  catch (const std::exception &e)
  {
    Serial.println("Hata .!");
    Serial.println(e.what());
    jDoc.clear();
    jDoc["error"] = e.what();
    serializeJson(jDoc, buffer);
    webServ.send(HTTP_INTERNAL_ERROR, CONTENT_TYPE, buffer);
  }
}

void ConfigurateRouting()
{
  webServ.on("/getStatus", GetStatus);
  webServ.on("/start", HTTP_POST, Start);
  webServ.on("/stop", HTTP_POST, Stop);
}

void setup()
{
  ClearBuffer();
  Serial.begin(115200);
  Serial.println("Setup is starting");
  pinMode(ESP_PIN_RELAY, OUTPUT);
  digitalWrite(ESP_PIN_RELAY,HIGH);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  // isConnected = wifiMan.autoConnect("Remote Button Connector", "arda8585");
  WiFi.begin("Arda_wireless2G4", "Busifrebuyuk3");
  while (WiFi.status() != WL_CONNECTED)
  {
    BlinkBuiltinLed(500);
  }

  SwitchOnBuiltinLed(isConnected = true);

  if (isConnected)
    Serial.println("Wifi Connection established");
  else
    Serial.println("Wifi connection failed.!");

  ConfigurateRouting();
  webServ.begin();
  Serial.println("Web Server Started!");
  
}

void loop()
{
  webServ.handleClient();
}