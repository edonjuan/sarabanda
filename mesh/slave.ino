// slave - jaula
//com6 Jaula 

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "painlessMesh.h"
#include "HX711.h"

// HW
#define DHTTYPE DHT11
#define DHTPIN  13 
#define Botton  0
#define pir     5 
#define ldr     A0
#define pir_rate_min 1
#define LOADCELL_DOUT_PIN 14
#define LOADCELL_SCK_PIN  12

// NETWORK
#define   MESH_PREFIX     "ENVIODATOS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
// MASTER
#define COM4BLUE  3822969472

// Global variables
int  conteo;  //pir

short  REF_A, REF_B;// cambio de referencia 
int mayor_A=-999; // declaracion del numero mayor y menor de la celda A 
int menor_A=999; 

int mayor_B=-999; // declaracion del numero mayor y menor de la celda B 
int menor_B=999; 

int diferencia_A, diferencia_B, gr_A, gr_A2, gr_B, gr_B2;
int diferenciatotal_A, diferenciatotal_B;
 
// Objetct declaration
HX711 scale_A;
HX711 scale_B;
DHT dht (DHTPIN, DHTTYPE); //dht
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// Functions structure
//void set_tare();
void sendMessage(); // Prototype so PlatformIO doesn't compla
void IRAM_ATTR set_tare();
void IRAM_ATTR pir_action();
void scale_a(void);
void scale_b(void);

// Task declaration
Task taskSendMessage( TASK_MINUTE *pir_rate_min  , TASK_FOREVER, &sendMessage );

void sendMessage() {

  float t, h;
  int m, l, a, b;

  digitalWrite(LED_BUILTIN, !HIGH);
  
  // Temp & Hum
  t=dht.readTemperature();
  h=dht.readHumidity();  
  if (isnan(h) || isnan(t)) {
   Serial.println(F("Failed to read DHT!"));
  }

  // Mov
  m = conteo;
  conteo = 0;

  // Luz
  l = analogRead(ldr);

  // A_scale
  if(REF_A){
     diferenciatotal_A = 0;// si hay cambio de ref la diferencia se iguala a 0 
  }
  else{        
     diferenciatotal_A = mayor_A-menor_A; //si no hay cambio de ref se hace la diferencia 
  }

  if(diferenciatotal_A<0){
    a = 0;
  }
  else{
    a = diferenciatotal_A;
  } 
  mayor_A=-999;
  menor_A=999; 


  // A_scale
  if(REF_B){
     diferenciatotal_B = 0;// si hay cambio de ref la diferencia se iguala a 0 
  }
  else{        
     diferenciatotal_B = mayor_B-menor_B; //si no hay cambio de ref se hace la diferencia 
  }

  if(diferenciatotal_B<0){
    b= 0;
  }
  else{
    b = diferenciatotal_B;
  } 
  mayor_B=-999;
  menor_B=999; 

  // msg contruct
  String  msg = String (t) + "T";
  msg += String (h) + "H";
  msg += String (m) + "M";
  msg += String (l) + "L";
  msg += String (a) + "A";
  msg += String (b) + "B";
  Serial.println(msg);

   // msg += mesh.getNodeId();
  mesh.sendSingle (COM4BLUE, msg ); // manda mensaje a blu   
  digitalWrite(LED_BUILTIN, !LOW);
}
     

void setup() {

  // HW   
  Serial.begin(9600);// inicializacion del Serial 
  Serial.println("StartSlave_CONFIG"); 
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);
  pinMode(pir, INPUT);
  pinMode (Botton,INPUT);
  attachInterrupt(digitalPinToInterrupt(Botton),set_tare , RISING  );//se inicializan las taras en 0 
  attachInterrupt(digitalPinToInterrupt(pir), pir_action, RISING);

  dht.begin (); // inicializacion del sensor de temperatura y humedad
  set_tare();   // start scaleA & scaleB

  // SW
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  // Network
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
}
          
void loop() {
            // it will run the user scheduler as well
  mesh.update();
  // Lectura de celdas (rate = ?)    
  scale_a();
  scale_b();  
}

// Function declaration
void scale_a(void){

   // Lectura SCALE_A   
   delay(500); // wait mux
   scale_A.get_units(10);
   gr_A = scale_A.get_units(10);
   diferencia_A = gr_A2 - gr_A; //se sacan las diferencias de la celda   

   if ((diferencia_A>=0)&&(diferencia_A<5)) {
        if(gr_A>mayor_A){
        mayor_A=gr_A;
        }
        if(gr_A<menor_A){
        menor_A=gr_A;
        }
        
        REF_A=0; // no cambios bruscos
   }
   else if (diferencia_A>=5){
         mayor_A=-999;
         menor_A=999; 
         REF_A=1; // si cambios bruscos
    }
           
    else if(diferencia_A<-5){ // se quita mucho peso celda A-B
         mayor_A=-999;
         menor_A=999; 
         REF_A=1;
    }              
    gr_A2 = gr_A;  
}


void scale_b(void){

   // Lectura SCALE_B   
   delay(500); // wait mux
   scale_B.get_units(10);
   gr_B = scale_B.get_units(10);
   diferencia_B = gr_B2 - gr_B; //se sacan las diferencias de la celda   

   if ((diferencia_B>=0)&&(diferencia_B<5)) {
        if(gr_B>mayor_B){
        mayor_B=gr_B;
        }
        if(gr_B<menor_B){
        menor_B=gr_B;
        }
        
        REF_B=0; // no cambios bruscos
   }
   else if (diferencia_B>=5){
         mayor_B=-999;
         menor_B=999; 
         REF_B=1; // si cambios bruscos
    }
           
    else if(diferencia_B<-5){ // se quita mucho peso celda A-B
         mayor_B=-999;
         menor_B=999; 
         REF_B=1;
    }              
    gr_B2 = gr_B;  
}


void IRAM_ATTR set_tare() { //interrupcion para inicializar taras en 0 

    Serial.println("SCALES_adjust");
    delay(1000);
  
    scale_A.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale_A.set_gain(128);
    scale_A.set_scale(850);
    scale_A.get_units(10);
    scale_A.tare();
  
    delay(1000);
  
    scale_B.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale_B.set_gain(32);
    scale_B.set_scale(500);
    scale_B.get_units(10);
    scale_B.tare();

    // Clear control variables
    diferencia_A=0; 
    diferencia_B=0; 
}

void IRAM_ATTR pir_action() {
  conteo++;    
}