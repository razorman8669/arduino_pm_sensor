#include <ESP8266WiFi.h>
#include <Arduino.h>
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

// Modified from https://how2electronics.com/iot-air-pollution-monitoring-esp8266/
// Make sure to read the installation for ESP8266 board here https://arduino-esp8266.readthedocs.io/en/latest/installing.html#instructions and choose nodeMCU 1.0 board

int PM01Value = 0;        //define PM1.0 value of the air detector module
int PM2_5Value = 0;       //define PM2.5 value of the air detector module
int PM10Value = 0;       //define PM10 value of the air detector module

// sample rate (in milliseconds) to send to thingspeak.  the free version allows for 1 message every 15 seconds.
int sample_rate = 15200;
int led_threshold = 12;

String apiKey = "<REPLACE WITH YOUR KEY>"; //Write API Key

const char* ssid = "<WIFI_SSID>"; //SSID name of wifi
const char* password = "<WIFI_PASSWORD>"; //password login of wifi

const char* server = "api.thingspeak.com";
WiFiClient client;

void setup()
{
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  if (Serial.find(0x42)) {  //start to read when detect 0x42
    Serial.readBytes(buf, LENG);

    if (buf[0] == 0x4d) {
      if (checkValue(buf, LENG)) {
        PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value = transmitPM2_5(buf); //count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }

  static unsigned long OledTimer = millis();
  if (millis() - OledTimer >= sample_rate) //sample frequency sent to thingspeak
  {
    // Light up LED if pm2.5 above threshold
    if (PM2_5Value >= led_threshold) {
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    OledTimer = millis();
    delay(10);
    Serial.print("PM1.0: ");
    Serial.print(PM01Value);
    Serial.println("  ug/m3");

    Serial.print("PM2.5: ");
    Serial.print(PM2_5Value);
    Serial.println("  ug/m3");

    Serial.print("PM10 : ");
    Serial.print(PM10Value);
    Serial.println("  ug/m3");
    Serial.println();

    if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
    {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(PM01Value);
      postStr += "&field2=";
      postStr += String(PM2_5Value);
      postStr += "&field3=";
      postStr += String(PM10Value);
      postStr += "\r\n\r\n";

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
    }
    client.stop();


  }

}
char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;

  for (int i = 0; i < (leng - 2); i++) {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}
int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}
//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); //count PM2.5 value of the air detector module
  return PM2_5Val;
}
//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}
