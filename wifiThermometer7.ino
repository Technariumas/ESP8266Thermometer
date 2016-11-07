#include <ESP8266WiFi.h>
#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Phant.h>

//AP definitions
#define AP_SSID "change_me"
#define AP_PASSWORD "change_me"

const char *sparkPhantHost = "data.sparkfun.com";
const char *sparkPublicKey = "change_me";
const char *sparkPrivateKey = "change_me";

const char *txPhantHost = "change_me";
const char *txPublicKey = "change_me";
const char *txPrivateKey = "change_me";

#define REPORT_INTERVAL_SECONDS 60 // in sec

#define ONE_WIRE_BUS D3  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

void wifiConnect();
int postToPhant(float temp, const char* PhantHost, const char* PublicKey, const char* PrivateKey );

void setup() {
  Serial.begin(115200);
  
  wifiConnect();
  pinMode(D2, OUTPUT);
  digitalWrite(D2, LOW);
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  
}

void loop() {
  float temp;
  
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.println(temp);
  } while (temp == 85.0 || temp == (-127.0));
  
  postToPhant(temp, sparkPhantHost, sparkPublicKey, sparkPrivateKey);
  postToPhant(temp, txPhantHost, txPublicKey, txPrivateKey);
  
  int cnt = REPORT_INTERVAL_SECONDS;
  
  while(cnt--)
    delay(1000);
}

void wifiConnect() {
  Serial.print("Connecting to AP");
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
}


int postToPhant(float temp, const char* PhantHost, const char* PublicKey, const char* PrivateKey ) {
  // LED turns on when we enter, it'll go off when we 
  // successfully post.
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("posting data to ");
  Serial.println(PhantHost);

  // Declare an object from the Phant library - phant
  Phant phant(PhantHost, PublicKey, PrivateKey);

//  String postedID = "Technarium-Outside-Air";

  // Add the four field/value pairs defined by our stream:
  phant.add("temperature", temp);
  phant.add("timestamp", millis());

  // Now connect to data.sparkfun.com, and post our data:
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(PhantHost, httpPort)) 
  {
    Serial.print("Failed to connect to ");
    Serial.println(PhantHost);
    // If we fail to connect, return 0.
    return 0;
  }
  // If we successfully connected, print our Phant post:
  client.print(phant.post());

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line); // Trying to avoid using serial
  }

  // Before we exit, turn the LED off.
  digitalWrite(LED_BUILTIN, LOW);

  return 1; // Return success
}

