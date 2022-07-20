#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "h4SHfR69azTGC1R9TXqI9c3_y4QTgFif";
char ssid[] = "Testnet";
char pass[] = "akhilesh";

////////////V pins Define/////////////
#define Lower V0
#define Beat V1
#define Waterlevel V2
#define Mainsled V3
#define Tanklid V4
#define Mode V5
//////////////////////////////////////

///Support variables///
int prevbeat=-1,getbeat,lowoff=0; //for beat check of Lower
long duration;//for Water level
int distance;//for Water level
float level;//for Water level
int tankopen=0;//tank open
unsigned long tstart=0,tstop=0;//tank open
unsigned long rstart=0,flowrate=0;//flow rate
int prevdistance,rateflag=0;//flow rate
int pstat=2;//lower power
int MODE=0;//mode selection
///////////////////////

WidgetBridge lower(Lower);
BlynkTimer timer1,timer2;
WidgetLED mainsled(Mainsled);

BLYNK_CONNECTED()
{
  lower.setAuthToken("TKsH871RyuNNqCn_8MorYkGXzVEvkMVO");
  Blynk.notify("Connected");
  Blynk.virtualWrite(Tanklid,"Tanklid Closed");
  Blynk.syncVirtual(Mode);
}

////////////////////Beat/////////////////////

BLYNK_WRITE(Beat)
{
  getbeat=param.asInt();
  Serial.println(getbeat);
  if(lowoff==0)
 {
    Blynk.notify("Mains power has been restored");
    lowoff=1;
    pstat=1;
    mainsled.on();
 }
}

void beatcheck()
{ if((getbeat==prevbeat)&&(lowoff==1))
  {
    Blynk.notify("Looks like the mains power went off");
    lowoff=0;
    pstat=0;
    mainsled.off();
  }
  prevbeat=getbeat;
}

//////////////////////////////////////////////

//////////////////Water Level and Tank open////////////////

void checklevel()
{
  digitalWrite(D5, LOW);
  delayMicroseconds(2);
  digitalWrite(D5, HIGH);
  delayMicroseconds(10);
  digitalWrite(D5, LOW);
  duration = pulseIn(D3, HIGH);
  distance= (duration*0.034/2);
  distance-=5;
  Serial.println(distance);
  level=(float((16-distance))/16)*100;
  if(level<0)
  Blynk.virtualWrite(Waterlevel,0);
  else
  Blynk.virtualWrite(Waterlevel,int(level));
}

void waterlevel()
{
 checklevel();
 if(level<7 && level>0 && MODE==2 )
 {
  lower.virtualWrite(V2,1);
  //startrate();
 }
 else if(level>20 && MODE==2 )
 {
  lower.virtualWrite(V2,0);
  //startrate();
 }
 else if(level<-20 && tankopen==0)
 {
  Blynk.notify("Tank has been opened"); //tank open alert
  Blynk.virtualWrite(Tanklid,"Tanklid Opened");
  tankopen=1;
  tstart=millis();
 }
 //else if(level>0)
 //tankopen=0;

 if(tankopen==1 && level>0)
 {
  tstop=millis();
  tstop=(tstop-tstart)/1000;
  Blynk.notify("Tank closed. It was opened for: " + String(tstop) + "s");
  Blynk.virtualWrite(Tanklid,"Tanklid Closed");
  tankopen=0;
 }
}

//////////////////////////////////////////////

//////////////////Flow rate/////////////////
/*void startrate()
{
  if(rateflag==0)
  {
    rstart=millis();
    prevdistance=distance;
    rateflag=1;
    Serial.println("here");
  }
  if(prevdistance!=distance)
  {
    flowrate=millis();
    flowrate=(flowrate-rstart)/1000;
    rateflag=0;
    Serial.println("Flow rate= "+ String(flowrate) + "s/cm");
  }
  
}*/
/////////////////////////////////////////////

////////////////Mode selection///////////////
BLYNK_WRITE(Mode)
{
  MODE=param.asInt();
  
}
////////////////////////////////////////////

void setup()
{
  Serial.begin(9600);
  OTA();
  Blynk.begin(auth, ssid, pass);
  timer1.setInterval(3*1000, beatcheck);
  timer2.setInterval(3*1000, waterlevel);
  pinMode(D5, OUTPUT);// For Water level 
  pinMode(D3, INPUT);// For Water level
}

void loop()
{
  Blynk.run();
  ArduinoOTA.handle();
  timer1.run();
  timer2.run();
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

  ArduinoOTA.setHostname("Pravah(U)");
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
