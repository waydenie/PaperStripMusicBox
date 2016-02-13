#include <synth.h>

byte muxValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
volatile boolean triggered;

ISR (ANALOG_COMP_vect) {
  triggered = true;
}

synth psmp;

void setup() {
  Serial.begin(9600);

  psmp.begin();

  psmp.setupVoice(0,SINE,60,ENVELOPE0,80,64);
  psmp.setupVoice(1,SINE,62,ENVELOPE0,100,64);
  psmp.setupVoice(2,SINE,64,ENVELOPE2,110,64);
  psmp.setupVoice(3,SINE,67,ENVELOPE0,110,64);

//Configure PORTD so that PD2-PD5 are outputs for selecting the external 16 ch multiplexer address and
//PD6(AIN0) and PD7(AIN1) are pos and neg inputs (resp.) to internal analog comparator
//Leave PD0-PD1 alone since we don't want to interfere with RX/TX
  DDRD  |= B00111100; // Set PD2-PD5 to outputs for mux channel address S0-S3
  DDRD  &= B00111111; // Set PD6-PD7 to inputs for comparator
  PORTD &= B00111111; // Disable internal pullups on PD6-PD7 so we don't interfere with the comparator

  ADCSRB = 0; // Disable ACME (Analog Comparator Multiplexer Enable)
  ACSR =
    (0 << ACD) |    // Analog Comparator: Enabled
    (0 << ACBG) |   // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
    (1 << ACO) |    // Analog Comparator Output: On
    (1 << ACI) |    // Analog Comparator Interrupt Flag: Clear Pending Interrupt
    (1 << ACIE) |   // Analog Comparator Interrupt: Enabled
    (0 << ACIC) |   // Analog Comparator Input Capture: Disabled
    (0 << ACIS1) | (0 << ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Toggle

Serial.println("Begin...");
}

void displayData()
// dumps captured data from array to serial monitor
{
  Serial.println();
  Serial.println("Values from multiplexer:");
  Serial.println("========================");
  for (int i = 0; i < 16; i++)
  {
    Serial.print("input I"); 
    Serial.print(i); 
    Serial.print(" = "); 
    Serial.println(muxValues[i]);
  }
  Serial.println("========================");  
}

void loop() {

  for (byte ch=0; ch<16; ch++) {
    PORTD &= B11000011; // Clear PD2-PD5 mux channel address without disturbing PD0-PD1 (RX/TX) or PD6-PD7 (comparator inputs)
    PORTD |= (ch << 2); // Set new mux channel address 
    delay(6); //scan inputs at approx 10 times/s
    muxValues[ch]=(ACSR & (1 << ACO));
  }
  displayData();

  if (triggered) {
    Serial.println ("Triggered!");
    triggered = false;
  }

  delay(1000);
}

