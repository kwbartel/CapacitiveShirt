// Fun with capacitive sensing and some machine code - for the Arduino (or Wiring Boards).
// Note that the machine code is based on Arduino Board and will probably require some changes for Wiring Board
// This works with a high value (1-10M) resistor between an output pin and an input pin.
// When the output pin changes it changes the state of the input pin in a time constant determined by R * C
// where R is the resistor and C is the capacitance of the pin plus any capacitance present at the sensor.
// It is possible when using this setup to see some variation in capacitance when one's hand is 3 to 4 inches from the sensors
// Try experimenting with larger sensors. Lower values of R will probably yield higher reliability.
// Use 1 M resistor (or less maybe) for absolute touch to activate.
// With a 10 M resistor the sensor will start to respond 1-2 inches away

// Setup
// Connect a 10M resistor between pins 8 and 9 on the Arduino Board
// Connect a small piece of alluminum or copper foil to a short wire and also connect it to pin 9

// When using this in an installation or device it's going to be important to use shielded cable if the wire between the sensor is 
// more than a few inches long, or it runs by anything that is not supposed to be sensed. 
// Calibration is also probably going to be an issue.
// Instead of "hard wiring" threshold values - store the "non touched" values in a variable on startup - and then compare.
// If your sensed object is many feet from the Arduino Board you're probably going to be better off using the Quantum cap sensors.

// Machine code and Port stuff from a forum post by ARP  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1169088394/0#0


int  i;
unsigned int x, y;
float accum, fout, fval = .07;    // these are variables for a simple low-pass (smoothing) filter - fval of 1 = no filter - .001 = max filter
float calib;
int people = 0;
// float baseline[30];
float baseline;
float cap;
int thresh;
float average[] = {0, 0, 0, 0, 0};
int led[] = {B00000000, B00000100, B00001100, B00011100, B00111100}; // 

void setup()
{
 Serial.begin(9600);
 DDRB=B101;
 DDRD = DDRD | B11111100;
 digitalWrite(10, LOW);  //could also be HIGH - don't use this pin for changing output though 
 
 Serial.println("---People Detector!---");
 
 thresh = 3;
 trashValues(500);
 baseline = newBaseLine();
 fillAverage(baseline);
 delay(200);
 Serial.println("Calib: " + (String)(int)baseline);
 delay(1000);
 
 for (int i = 0; i < sizeof(led); i++)
 {
   PORTD = led[i];
   delay(1000);
 }
}

void loop()
{   
  updateLED();
  
  cap = getCap();
  updateAverage(cap);
  float avg = calcAverage();
  delay(40);
  
  Serial.println("Cap: " + (String)(int)cap + "\n" + "Avg: " + (String)(int)avg);
  
  if (avg - baseline > thresh)
  {
    Serial.println("PERSON DETECTED");
    trashValues(100);    
    people++;
    baseline = newBaseLine(); 
    fillAverage(baseline);
    Serial.println("People: " + (String)people + "\n");
  }
  
  //Assumes peole let go in order? Not anymore!
  else if (baseline - avg > thresh)
  {
    people--;
    Serial.println("PERSON LOST"); 
    delay(40);
    trashValues(100);
    baseline = newBaseLine();
    fillAverage(baseline);
    Serial.println("People: " + (String)people + "\n");
  } 

   delay(20);
   
}

void updateLED()
{
   if (people < 0)
    PORTD = led[0];
  else if (people > 4)
    PORTD = led[4];
  else
    PORTD = led[people];
}

void fillAverage(float f)
{
   for (int i = 0; i < 5; i++)
   {
     average[i] = f;
     //Serial.println((String)(int)average[i]);
   }
}

void updateAverage(float c)
{  
  for (int i = 0; i < 5 - 1; i++)
    average[i] = average[i + 1];

  average[4] = c;
}

float calcAverage()
{
  float avg;
  for (int i = 0; i < 5; i++)
  {
    avg += average[i];
    //Serial.println((String)i + " : " + (String)(int)avg);
  }
  avg /= 5.0;
  //Serial.println((String)(int)avg);
  return avg;
}

float newBaseLine()
{
  float base = 0;
  for (int i = 0; i < 5; i++)
    base += getCap();  
  return base /= 5.0;
}

void trashValues(int t)
{
   for (int i = 0; i < t; i++)
     getCap();
}

float getCap()
{
  y = 0;        // clear out variables
  x = 0;

 for (i=0; i < 4 ; i++ ){       // do it four times to build up an average - not really neccessary but takes out some jitter

   // LOW-to-HIGH transition
   PORTB = PORTB | 1;                    // Same as line below -  shows programmer chops but doesn't really buy any more speed
   // digitalWrite(8, HIGH);    
   // output pin is PortB0 (Arduino 8), sensor pin is PortB1 (Arduinio 9)                                   

   while ((PINB & B10) != B10 ) {        // while the sense pin is not high
     //  while (digitalRead(9) != 1)     // same as above port manipulation above - only 20 times slower!                
     x++;
   }
   delay(1);

   //  HIGH-to-LOW transition
   PORTB = PORTB & 0xFE;                // Same as line below - these shows programmer chops but doesn't really buy any more speed
   //digitalWrite(8, LOW);              
   while((PINB & B10) != 0 ){            // while pin is not low  -- same as below only 20 times faster
     // while(digitalRead(9) != 0 )      // same as above port manipulation - only 20 times slower!
     y++;  
   }

   delay(1);
 }

 fout =  (fval * (float)x) + ((1-fval) * accum);  // Easy smoothing filter "fval" determines amount of new data in fout
 accum = fout;
 return fout;
}
