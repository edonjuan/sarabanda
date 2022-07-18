// slave Mesh
// Temperatura, Humedad, Movimiento y Luz


#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "painlessMesh.h"

// HW
#define DHTTYPE DHT11
#define DHTPIN  13 
#define Botton  0
#define pir     5 
#define ldr     A0
#define msg_rate 10

// NETWORK
#define   MESH_PREFIX     "ENVIODATOS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
// MASTER
#define MASTER  3822969472

// Global variables
unsigned int conteo;  //pir
unsigned short check;
unsigned short tenSec;

// Objetct declaration
DHT dht (DHTPIN, DHTTYPE); //dht
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// Functions structure
//void set_tare();
void sendMessage(); // Prototype so PlatformIO doesn't compla
void IRAM_ATTR pir_action();

// Task declaration
Task taskSendMessage( TASK_SECOND *msg_rate  , TASK_FOREVER, &sendMessage );

void sendMessage() {

  float t, h;
  int l;
  unsigned int m;

  if(tenSec>=5)
  {
    tenSec=0;
    check = 0;
  }
  else
  {
    tenSec++;
  }
  Serial.print("iteracion: ");
  Serial.println(tenSec);

  if(check == 0)
  {  
      digitalWrite(LED_BUILTIN, !HIGH);
      
      // Temp & Hum
      t=dht.readTemperature();
      h=dht.readHumidity();  
      if (isnan(h) || isnan(t)) {
       Serial.println(F("Failed to read DHT!"));
      }
    
      // Mov
      m = conteo;
    
      // Luz
      l = analogRead(ldr);
    
      // msg contruct
      String  msg = String (t) + "T";
      msg += String (h) + "H";
      msg += String (m) + "M";
      msg += String (l) + "L";

      Serial.println(msg);
      mesh.sendSingle (MASTER, msg); // manda mensaje a blu 
      digitalWrite(LED_BUILTIN, !LOW);
  }
}
     

void setup() {

  // HW   
  Serial.begin(9600);// inicializacion del Serial 
  Serial.println("StartSlave_CONFIG"); 
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);
  pinMode(pir, INPUT);
  pinMode (Botton,INPUT);
  attachInterrupt(digitalPinToInterrupt(pir), pir_action, RISING);

  dht.begin (); // inicializacion del sensor de temperatura y humedad

  // SW
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  // Network
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
}
          
void loop() {
  mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  if(from == MASTER)
  {
    check = 1;
    tenSec=0;
  }
}

void IRAM_ATTR pir_action() {
  conteo++;    
}