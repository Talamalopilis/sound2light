#include <EEPROM.h>

struct colour{
  int r;
  int g;
  int b;
};

  int redPin = 9;   // Red LED,   connected to digital pin 9
  int grnPin = 10;  // Green LED, connected to digital pin 10
  int bluPin = 11;  // Blue LED,  connected to digital pin 11
  
  colour RED = {255,0,0};
  colour VIOLET = {148,0,211};
  colour WHITE = {100,100,255};
  colour YELLOW = {255,255,0};
  colour colour_cycles [4]= {RED, VIOLET, WHITE, YELLOW}; // colour states, will be cycled with reset button
  int colour_state;
  int TOTAL_COLOURS = 4;
  
  int analogPin = 0; // read from multiplexer using analog input 0
  int strobePin = 2; // strobe is attached to digital pin 2
  int resetPin = 3; // reset is attached to digital pin 3
  int spectrumValue[7]; // to hold a2d values

void setup() {
  // The following code block will handle initialising pins and
  //reading/setting the colour state using the eeprom.
  EEPROM.get(0,colour_state);
  if (colour_state > TOTAL_COLOURS || colour_state < 0)
    colour_state = 0;


   Serial.begin(9600);
   pinMode(redPin, OUTPUT);
   pinMode(grnPin, OUTPUT);
   pinMode(bluPin, OUTPUT);
   pinMode(analogPin, INPUT);
   pinMode(strobePin, OUTPUT);
   pinMode(resetPin, OUTPUT);
   analogReference(DEFAULT);
  
   digitalWrite(resetPin, LOW);
   digitalWrite(strobePin, HIGH);
  
   Serial.print(colour_state);
   
   if (colour_state < TOTAL_COLOURS)
     colour_state = colour_state + 1;
   else 
     colour_state = 0;
   EEPROM.put(0,colour_state);
}

void loop()
{
 digitalWrite(resetPin, HIGH);
 digitalWrite(resetPin, LOW);
 
 // The following for loop will read input from the IC depicting a 
 // fourier transform to a resolution of 7 points
 for (int i = 0; i < 7; i++)
 {
 digitalWrite(strobePin, LOW);
 delayMicroseconds(30); // to allow the output to settle
 spectrumValue[i] = analogRead(analogPin);
 digitalWrite(strobePin, HIGH);
 }
 Serial.println();
 
  // The following code block calculates the centroid of the plot in spectrumValue.
  float lower,higher,arect,xrect,yrect,atri,xtri,ytri;
  float totalarea=0, totalx=0, totaly=0;
  for (int i=0;i<6;i++){
    lower = min(spectrumValue[i],spectrumValue[i+1]);
    higher = max(spectrumValue[i],spectrumValue[i+1]);
    arect = lower;
    xrect = i + 0.5;
    yrect = lower/2;
    
    atri = (higher-lower)/2;
    if (spectrumValue[i] > spectrumValue[i+1])
      xtri = i + 1/3;
    else
      xtri = i + 2/3;
    ytri = (higher-lower)/3 + lower;
    
    totalarea= totalarea + arect + atri;
    totalx = totalx + xrect*arect + xtri*atri;
    totaly = totaly + yrect*arect + ytri*atri;
    }
    float x = totalx/totalarea;
    float y = totaly/totalarea;

  // The following code block converts the centroid values into red, green, and blue coordinates.
  float xratio = x/6;
  float yratio = y/512;
  float red,green,blue;
  if (xratio < 0.15){
    red = -(xratio-0.15)/0.15;
    green = 0;
    blue = 1;
  }
  else if (xratio < 0.275 ){
    red = 0;
    green = (xratio-0.15)/0.125;
    blue = 1;
  }
  else if (xratio < 0.325){
    red = 0;
    green = 1;
    blue = -(xratio-0.325)/0.05;
  }
  else if (xratio < 0.5){
    red = (xratio-0.325)/0.175;
    green = 1;
    blue = 0;
  }
  else if (xratio < 0.66){
    red = 1;
    green = -(xratio-0.66)/0.16;
    blue = 0;
  }
  else if (xratio < 1){
    red = 1.0;
    green = 0;
    blue = 0;
  }
  
  //fade at edges
  float factor = 1;
  if (xratio < 0.1){
    factor = 0.3 + 0.7*xratio/0.1;
  }
  else if(xratio > 0.8){
    factor = 0.3 + 0.7*(1-xratio)/0.2;
  }
  
  red = red*factor*yratio*255;
  green = green*factor*yratio*255;
  blue = blue*factor*yratio*255;
  
  int r = red + 0.5;
  int g = green + 0.5;
  int b = blue + 0.5;
  
  // The following code block pushes the red, green, and blue values 
  // to the control transistors, turning on the lights.
  if (colour_state != 0){
    Serial.println();
    Serial.print(colour_state);
    Serial.println();
    colour set = colour_cycles[colour_state-1];
    r = set.r;
    g = set.g;
    b = set.b;}
  
  Serial.print(r);\
   Serial.println();
  Serial.print(g);
   Serial.println();
  Serial.print(b);
   Serial.println();
   

  
  analogWrite(redPin,r);
  analogWrite(grnPin,g);
  analogWrite(bluPin,b);

}
