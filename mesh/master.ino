/* MESH_Master
*/

// Libraries
#include "painlessMesh.h"

// Define's
#define   MESH_PREFIX     "ENVIODATOS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// ID´s 
#define BRAVO    3822979383
#define CHARLY   2440612283 
#define DELTA    3257178212  
#define ECHO     3257177579 
#define MCU3      02440627835

#define MASTER  3822969472

// Objects
Scheduler userScheduler;
painlessMesh  mesh;

void setup() {  

  // HW
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);

  //mesh.setDebugMsgTypes( ERROR | STARTUP ); 
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
}

void loop() {
  mesh.update();
}

void receivedCallback(uint32_t from, String&msg ) {  
    float t, h;
    int l, a, b;
    unsigned int m;
    String sub_S;

    digitalWrite(LED_BUILTIN, !HIGH);

    Serial.print(from);
    Serial.print(",");
    if(msg.indexOf('T')>0)
    {
      sub_S = msg.substring(0,msg.indexOf('T'));
      t = sub_S.toFloat();
      Serial.print(t);
    }
    else
    {
      Serial.print("");
    }
        
    Serial.print (",");
    
    if(msg.indexOf('H')>0)
    {
      sub_S = msg.substring(msg.indexOf('T')+1,msg.indexOf('H'));
      h = sub_S.toFloat();
      Serial.print(h);
    }
    else
    {
      Serial.print("");
    }
         
    Serial.print (",");
    
    if(msg.indexOf('M')>0)
    {
      sub_S = msg.substring(msg.indexOf('H')+1,msg.indexOf('M'));
      m = sub_S.toInt();
      Serial.print(m);
    }
    else
    {
      Serial.print("");
    }
        
    Serial.print (",");
    
    if(msg.indexOf('L')>0)
    {
      sub_S = msg.substring(msg.indexOf('M')+1,msg.indexOf('L'));
      l = sub_S.toInt();
      Serial.print(l);
    }
    else
    {
      Serial.print("");
    }
   
    Serial.print (",");
    
    if(msg.indexOf('A')>0)
    {
      sub_S = msg.substring(msg.indexOf('L')+1,msg.indexOf('A'));
      a = sub_S.toInt();
      Serial.print(a);
    }
    else
    {
      Serial.print("");
    }

    Serial.print (",");

    if(msg.indexOf('B')>0)
    {
      sub_S = msg.substring(msg.indexOf('A')+1,msg.indexOf('B'));
      b = sub_S.toInt();         
      Serial.print(b);
    }
    else
    {
      Serial.print("");
    }
    
    Serial.print (",");
    Serial.println();

    digitalWrite(LED_BUILTIN, !LOW);    
}