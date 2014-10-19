// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <TimerOne.h>
#include "MPU6050.h"

#define LEDsPIN        9
#define lowBattPin     7
#define IMUintPin      2
#define NUMPIXELS      60
#define FILTERTAPS     100

// Standard Colors
#define RED            0xFF0000
#define GREEN          0x00FF00
#define BLUE           0x0000FF
#define YELLOW         0xAAAA00
#define LOWWHITE       0x333333
#define MIDWHITE       0x555555

// Init some things, neopixel LED lib, motion sensing raw data and converted data
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDsPIN, NEO_GRB + NEO_KHZ800);
extern IMUMotion Motion;
struct deviceAngle{float hyp, theta;};

// Global variables
int modeState = 0;
int framerate = 20;
float xinc = 0; // wondering x
float yinc = 3; // wondering y

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(2, INPUT);
  pinMode(lowBattPin, INPUT);
  //attachInterrupt(0, wake, CHANGE); // attach pin 2 as int0
  
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0); // bring MPU6050 out of sleep mode
  //MPU6050_write_reg (MPU6050_CONFIG, 0x04); // enable LPF interrupt
  //MPU6050_write_reg (MPU6050_INT_PIN_CFG, 0x14); // enable MOT_EN interrupt
  MPU6050_write_reg (MPU6050_INT_ENABLE, 0x40); // enable MOT_EN interrupt
}

int draw(struct deviceAngle *angle, int state){
  int x = 0;
  int y = 0;
  pixels.clear();
  
  switch(state){
     case 0: // turn off LEDs
      pixels.clear();
      break;
      
    case 1:
      x = (int)angle->theta;
      printPixel(x, 0, 0x111111);
      printPixel(x, 1, 0x555555);
      printPixel(x, 2, 0xFFFFFF);
      printPixel(x, 3, 0xFFFFFF);
      printPixel(x, 4, 0x555555);
      printPixel(x, 5, 0x111111);
      break;
      
    case 2:
      x = ((int)angle->theta) + 5;
      if(x >= 10) x -= 10;
      printPixel(x, 0, 0x111111);
      printPixel(x, 1, 0x555555);
      printPixel(x, 2, 0xFFFFFF);
      printPixel(x, 3, 0xFFFFFF);
      printPixel(x, 4, 0x555555);
      printPixel(x, 5, 0x111111);
      break;
      
    case 3:
      if(((int)angle->hyp) > 0){
        x = (int)angle->theta;
        y = (int)angle->hyp;
        printPixel(x, y, MIDWHITE);
        y +=random(-1,1);
        printPixel(x+1, y, YELLOW);
        printPixel(x-1, y, YELLOW);
      }
      else{ pixels.clear(); }
      break;
    
    case 4: // wondering fairy
      yinc += ((float)random(-1,2) / 10);
      Serial.println(yinc);
      xinc += 0.2;
      if(yinc > 5) yinc = 5;
      if(yinc < 1) yinc = 1;
      if(xinc >= 10) xinc -= 10;
      printPixel((int)xinc, (int)yinc, MIDWHITE);
      y = (int)yinc;
      x = (int)xinc;
      y +=random(-1,1);
      printPixel(x-1, y, YELLOW);
      break;
    
    case 5: // soft glow
      for(int i = 0; i <= 60; i++){
        pixels.setPixelColor(i,LOWWHITE);
      }
      break;
      
    case 6: // return to zero state
      state = 0;
      break;
  }  
  
  pixels.show();
  return state;
}

void printPixel(int x, int y, uint32_t color){
  int loc = y + (x * 6);
  pixels.setPixelColor(loc,color);
}

int IMUangle(struct deviceAngle *buffer, int mode){
  // Vectorise the tilt angle of the jar in polar space
  buffer->hyp = sqrt(square(Motion.z) + square(Motion.x));
  buffer->theta = asin(abs(Motion.z) / buffer->hyp);
  // Determine the angle quadrant to get a 2pi rotation value
  if((Motion.x < 0) && (Motion.z > 0)){  buffer->theta = ((PI*0.5)-buffer->theta) + (PI*0.5);  }
  if((Motion.x < 0) && (Motion.z < 0)){  buffer->theta += PI;}
  if((Motion.x > 0) && (Motion.z < 0)){  buffer->theta = ((PI*0.5)-buffer->theta) + (PI*1.5);  }
  
  // normalise hyp and theta
  buffer->hyp =   buffer->hyp * 4;
  buffer->theta = buffer->theta / (2*PI);
  
  // custom scaling factors
  buffer->hyp =   buffer->hyp * 5;
  buffer->theta = buffer->theta * 10;
  
  // mode change
  if(Motion.y > 0.4){
    mode += 1;
    delay(200); 
  }
  return mode;
}

void loop() {
  struct deviceAngle angle = {0,0};
  readImu();
  modeState = IMUangle(&angle, modeState);
  if(!digitalRead(lowBattPin)){  modeState = 0;  }
  modeState = draw(&angle, modeState);
  delay(framerate);
}

void wake(){
  Serial.println("'Oh?'");
  delay(500);
}


