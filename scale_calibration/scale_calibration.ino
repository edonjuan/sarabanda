#include "HX711.h"

const int LOADCELL_DOUT_PIN = 14;
const int LOADCELL_SCK_PIN = 12;

HX711 scale_a;
HX711 scale_b;

void setup() {
  Serial.begin(9600);
  Serial.println("Sarabanda:  SCALE CALIBRATION");

  Serial.println("Press number 1 ... ");

  scale_a.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale_a.set_gain(128);
  scale_a.set_scale(889);
  scale_a.tare();

  delay(1000);

  scale_b.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale_b.set_gain(32);
  scale_b.set_scale(508);
  scale_b.tare();
}

void loop() {

   if (Serial.available() > 0) {
        // read the incoming byte:
        int incomingByte = Serial.read();
        
        
        if(incomingByte == 49)
        {
          
          scale_a.get_units(1);
          Serial.print("Scale_A: \t");
          int gr_a = scale_a.get_units(10);
          Serial.print(gr_a);
          Serial.print(" gr");

          scale_b.get_units(1);
          Serial.print("\t Scale_B: \t");
          int gr_b = scale_b.get_units(10);
          Serial.print(gr_b);
          Serial.println(" gr");
          
        }
    }
}
