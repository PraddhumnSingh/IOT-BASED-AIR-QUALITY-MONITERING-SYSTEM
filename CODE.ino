#include <ESP8266WiFi.h>
#include <GP2YDustSensor.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 13      //Pin D7
#define R 14          //Pin D5
#define G 15         //Pin D8
#define B 12        //Pin D6

const uint8_t SHARP_LED_PIN = 2;   // Sharp Dust/particle sensor Led Pi---Pin D4
const uint8_t SHARP_VO_PIN = A0;    // Sharp Dust/particle analog out pin used for reading 

#define DHTTYPE    DHT11     // DHT 11

GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1010AU0F, SHARP_LED_PIN, SHARP_VO_PIN);
DHT dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
int PM2_5VALUE = 0;

float h,t,f,hic,hif;

String apiKey = "--------------";

const char* ssid = "-----------";   //your ssid(hostpot name)
const char* pass = "-----------";  //your ssid pasword

const char* server = "api.thingspeak.com";
WiFiClient client;

void setup() {
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  digitalWrite(R, 1);
  digitalWrite(G, 1);
  digitalWrite(B, 1);

  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  //dustSensor.setBaseline(0.4); // set no dust voltage according to your own experiments
  //dustSensor.setCalibrationFactor(1.1); // calibrate against precision instrument
  dustSensor.begin();



  // Initialize device.
  dht.begin();
  
}

void loop()
{
 API(); 
 AQI();
 HT();
 

 
 if(PM2_5VALUE > -1 && PM2_5VALUE  < 30)
 {
   analogWrite(R, 900);
   analogWrite(G,00);
   analogWrite(B, 1000); 
 }
 else if(PM2_5VALUE > 30 && PM2_5VALUE < 60)
 {
   analogWrite(R, 1024);
   analogWrite(G, 800);
   analogWrite(B, 1024);
 }
 else if(PM2_5VALUE > 60 && PM2_5VALUE < 90)
 {
   analogWrite(R, 0);
   analogWrite(G, 850);
   analogWrite(B, 1024); 
 }
 else if(PM2_5VALUE > 90 && PM2_5VALUE < 120)
 {
   analogWrite(R, 0);
   analogWrite(G, 990);
   analogWrite(B, 1024); 
 }
else if(PM2_5VALUE > 120 && PM2_5VALUE < 250)
 {
   analogWrite(R, 0);
   analogWrite(G, 1020);
   analogWrite(B, 1024);
 }
 else if(PM2_5VALUE > 250)
 {
   analogWrite(R, 0);
   analogWrite(G, 1024);
   analogWrite(B, 1024); 
 }
  

}

void AQI()
 { 
   
   PM2_5VALUE = dustSensor.getRunningAverage();
   Serial.print("Dust density: ");
   Serial.print(dustSensor.getDustDensity());
   Serial.print(" µg/m3; Running average: ");
   Serial.print(dustSensor.getRunningAverage());
   Serial.println(" µg/m3"); 
  
   Serial.print("");    
   delay(2000);
 }

void HT()
{
   // Wait a few seconds between measurements.
   delay(2000);
   // Reading temperature or humidity takes about 250 milliseconds!
   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
   h = dht.readHumidity();
   // Read temperature as Celsius (the default)
   t = dht.readTemperature();
   // Read temperature as Fahrenheit (isFahrenheit = true)
   f = dht.readTemperature(true);

   // Check if any reads failed and exit early (to try again).
   if (isnan(h) || isnan(t) || isnan(f))
   {
     Serial.println(F("Failed to read from DHT sensor!"));
     return;
   }

   // Compute heat index in Fahrenheit (the default)
   hif = dht.computeHeatIndex(f, h);
   // Compute heat index in Celsius (isFahreheit = false)
   hic = dht.computeHeatIndex(t, h, false);

   Serial.print(F("Humidity: "));
   Serial.print(h);
   Serial.print(F("%  Temperature: "));
   Serial.print(t);
   Serial.print(F("°C "));
   Serial.print(f);
   Serial.print(F("°F  Heat index: "));
   Serial.print(hic);
   Serial.print(F("°C "));
   Serial.print(hif);
   Serial.println(F("°F"));
}

void API()
{
  if (client.connect(server,80))  // "184.106.153.149" or api.thingspeak.com
   {
        String postStr = apiKey;
        postStr +="&field1=";
        postStr += String(PM2_5VALUE);
        postStr +="&field2=";
        postStr += String(hic);
        postStr +="&field3=";
        postStr += String(h);
        postStr += "\r\n\r\n";
        
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr); 
    }   
  client.stop();  
  delay(500);   
}

