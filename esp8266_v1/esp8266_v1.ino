// Sarabanda: esp8266_vi file
// NOTE: ESP8266 Library version 2.5.0
// Author: git@edonjuan

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
  pinMode(DHTPIN, INPUT_PULLUP);
  
  pinMode(PIR, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR), pir_action, RISING);

  dht.begin(); 
  setup_loadcell();
  Serial.print("\n\n\n****Start****\n");
}

// Main
void loop() {

  // Rate
  delay(5000);

  // Heartbeat
  digitalWrite(LED, !HIGH);

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
  Serial.println(scale.get_units(10), 1);

  scale.power_down();    
  Serial.print("\n\n");
  digitalWrite(LED, !LOW); 
          
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

