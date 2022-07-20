#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "TKsH871RyuNNqCn_8MorYkGXzVEvkMVO";
char ssid[] = "Testnet";
char pass[] = "akhilesh";

////////////V pins Define/////////////
#define Upper V0
#define Beat V1
#define Motor V2
#define Motorswitch V3
#define Rain V4
#define Leak V5
#define Foot V6
//////////////////////////////////////

///Support variables///
int beat=0; //for beat check
int motorflag=0;
int Switch=0;//manual operation
int dat;
///////////////////////

WidgetBridge upper(Upper);
BlynkTimer timer;

BLYNK_CONNECTED()
{
  upper.setAuthToken("h4SHfR69azTGC1R9TXqI9c3_y4QTgFif");
}

////////////beat check/////////////////

void sendbeat()
{
  beat=++beat;
  if(beat>100)
  beat=0;
  upper.virtualWrite(Beat,beat); 
}

///////////////////////////////////////

////////////////motor//////////////////
BLYNK_WRITE(Motor)
{
  int dat=param.asInt();
  if(dat==1 && motorflag==0)
  {
    digitalWrite(D3,HIGH);
    Blynk.virtualWrite(Motorswitch,1);
    Blynk.notify("Tank started refilling");
    motorflag=1;
  }
  if(dat==0 && motorflag==1)
  {
    digitalWrite(D3,LOW);
    Blynk.virtualWrite(Motorswitch,0);
    Blynk.notify("Tank fully refilled");
    motorflag=0;
  }
}
//////////////////////////////////////

////////////////Motor manual////////////
BLYNK_WRITE(Motorswitch)
{
 Switch=param.asInt();
 if(Switch)
 digitalWrite(D3,HIGH);
 else
 digitalWrite(D3,LOW);
}

///////////////////////////////////////

BLYNK_WRITE(Rain)
{
  dat=param.asInt();
  if(dat)
  Blynk.notify("Looks like its gonna rain at your home");
}

BLYNK_WRITE(Leak)
{
  dat=param.asInt();
  if(dat)
  Blynk.notify("Water seems to be leaking at your home");
}

BLYNK_WRITE(Foot)
{
  dat=param.asInt();
  if(dat)
  Blynk.notify("Motor Stopped.Please check the footvalve!");
}

void setup()
{ 
  pinMode(D3,OUTPUT);
  Serial.begin(9600);
  OTA();
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1*1000, sendbeat);
  digitalWrite(D3,LOW);
}

void loop()
{
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}





void OTA()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("Pravah(L)");
  ArduinoOTA.setPassword((const char *)"sensepro");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}
