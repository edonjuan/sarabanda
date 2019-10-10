// Sarabanda: esp8266_vi file
// NOTE: ESP8266 Library version 2.5.0
// Author: git@edonjuan

// Connection libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "Intel-IoT";
const char* password = "1N73RN37D3L45C0545";
const char* mqtt_server = "edonjuan.duckdns.org";
const char* mqtt_user = "mqtt_user";
const char* mqtt_pass = "mqtt_pass";

WiFiClient espClient;
PubSubClient client(espClient);

// DHT file & Configuration
#include "DHT.h"
#define DHTPIN 10 
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

// HX711 file & Configuration
#include "HX711.h"
#define LOADCELL_DOUT_PIN 14
#define LOADCELL_SCK_PIN 12
HX711 scale;

// NodeMCU ports
#define LED 16
#define PIR 5
#define LDR A0

// Functions definitions
void setup_loadcell(void);
void setup_wifi(void);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect(void);

// Variables definitions
int pir_counter=0;

// ISR_estructure
void pir_action() {
  pir_counter++;  
}

// Configuration
void setup() {

  Serial.begin(9600);
  Serial.println("Sarabandav1");
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, !LOW);
  pinMode(DHTPIN, INPUT_PULLUP);
  
  pinMode(PIR, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR), pir_action, RISING);

  dht.begin(); 
  setup_loadcell();
  setup_wifi();

  client.setClient(espClient);
  client.setServer(mqtt_server, 1883);

  // Function to call (Data incoming)
  client.setCallback(callback);
  
  Serial.print("\n\n\n****Start****\n");
}

// Main
void loop() {

   if (!client.connected()) {
    reconnect();
  }
  client.loop();  

  // Rate
  delay(5000);

  // DHT11 sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }
  else {
    Serial.print("Humidity: \t");
    Serial.print(h);
    Serial.println("%");
    Serial.print("Temperature: \t");
    Serial.print(t);
    Serial.println("°C ");
  }
 
  // PIR sensor
  Serial.print("PIR Detection: \t");
  Serial.println(pir_counter);

  // LDR sensor
  int ldr_value = analogRead(LDR);
  Serial.print("LDR Value: \t");
  Serial.println(ldr_value);

  //scale.set_gain(64);
  scale.power_up();
  //Serial.print("one reading:\t");
  //Serial.print(scale.get_units(), 1);
  Serial.print("scale_ch_A: \t");
  float scale_a = scale.get_units(10);
  
  Serial.println(scale_a, 2);

  scale.power_down();    
  Serial.print("\n\n");

  // Send data to HASS
  sendmqtt("egi/humidity", h);
  sendmqtt("egi/temperature", t);
  sendmqtt("egi/pir", pir_counter);
  sendmqtt("egi/ldr", ldr_value);
  sendmqtt("egi/scale_a", scale_a);
  sendmqtt("egi/scale_b", 5);          
}

void setup_loadcell(void)
{
    // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  //scale.set_gain(64);
  

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)

  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");  
}

void setup_wifi() {  
 
  // We start by connecting to a WiFi network
  Serial.println();
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  String strMessage;
  String strTopic;
  String strPayload;
 
  payload[length] = '\0';
  strTopic = String((char*)topic);
  strMessage = String((char*)payload);
  //Serial.println(strTopic);
  //Serial.println(strMessage);
}

void sendmqtt(char* topic, float data)
{
  String data_str;
  char data_chr[50];
  int data_int;

  data_int = (int)data;
  if(data_int == data)
  {
    data_str = String(data_int);
  }
  else
  {
    data_str = String(data);
  }
  data_str.toCharArray(data_chr, data_str.length() + 1);
  client.publish(topic, data_chr);  
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("egiProject", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      digitalWrite(LED, !HIGH);
      // Once connected, publish an announcement...
      client.subscribe("egi/ctrl");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(LED, !LOW);
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
