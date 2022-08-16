/* MESH_Slave
 *  Sensor: Temperature / Humidity (DTH11/22)
 *  PIR, LDR (Analog)
 *  Watchdog for Master
*/

// Libraries
#include <DHT.h>
#include "painlessMesh.h"

// HW
#define DHTTYPE   DHT11
#define DHTPIN    14 
#define BTN       0
#define PIR       13 
#define LDR       A0

// SW
#define MASTER    3822969472
#define msg_rate  10
#define wtd_rate  10
#define wtd_rest  3

// NETWORK
#define   MESH_PREFIX     "ENVIODATOS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Objetct declaration
DHT dht (DHTPIN, DHTTYPE);
Scheduler userScheduler;
painlessMesh  mesh;

// Global variables
unsigned int mov_detections;

// Functions structure
void sendMessage(); 
void wtd();
void IRAM_ATTR pir_action();

// Task declaration
Task taskSendMessage(TASK_SECOND *msg_rate, TASK_FOREVER, &sendMessage);
Task taskWatchDog(TASK_SECOND *wtd_rate, TASK_FOREVER, &wtd);     

void setup() {

  // HW   
  Serial.begin(9600); 
  Serial.println("\nStart MESH SLAVE + WTD"); 
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);
  pinMode(PIR, INPUT);
  pinMode (BTN,INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR), pir_action, RISING);

  dht.begin ();

  // Network
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);

  // SW
  userScheduler.addTask(taskSendMessage);
  userScheduler.addTask(taskWatchDog);
  taskWatchDog.enable();
  taskSendMessage.enable();
}
          
void loop() {
  mesh.update();
}

void sendMessage() {

  float t=dht.readTemperature();
  float h=dht.readHumidity();  
  if (isnan(h) || isnan(t))
   Serial.println(F("Failed to read DHT!"));
   
  unsigned int m = mov_detections;
  short l = analogRead(LDR);
    
  // msg contruct
  String  msg = String (t) + "T";
  msg += String (h) + "H";
  msg += String (m) + "M";
  msg += String (l) + "L";

  Serial.println(msg);
  mesh.sendSingle (MASTER, msg);
}

void wtd(){
  static short watchCount;
  char strBuf[40];

  if(mesh.isConnected(MASTER))
  {
    digitalWrite(LED_BUILTIN, !HIGH);
    watchCount = 0;
  }
  else
  {
    digitalWrite(LED_BUILTIN, !LOW);
    watchCount++;
    if(watchCount>=wtd_rest) ESP.reset();
    sprintf(strBuf, "Master not found! Watchdog Count: %d of %d", watchCount, wtd_rest);
    Serial.println(strBuf);
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

void IRAM_ATTR pir_action() {
  mov_detections++;    
}