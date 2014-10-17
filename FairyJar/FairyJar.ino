// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "MPU6050.h"

#define LEDsPIN        9
#define lowBattPin     7
#define IMUintPin      2
#define NUMPIXELS      60
#define FRAMERATE      20
#define FILTERTAPS     100

#define RED            0xFF0000
#define GREEN          0x00FF00
#define BLUE           0x0000FF
#define YELLOW         0xAAAA00
#define SKIN           0xFFC4B2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDsPIN, NEO_GRB + NEO_KHZ800);

extern IMUMotion Motion;

struct deviceAngle{float hyp, theta;};

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pixels.begin(); // This initializes the NeoPixel library.
  attachInterrupt(0, wake, CHANGE); // attach pin 2 as int0
  pinMode(lowBattPin, INPUT);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0); // bring MPU6050 out of sleep mode
}

void draw(struct deviceAngle *angle){
  if(angle->hyp > 0){
    int pos =( angle->theta * 6) + (angle->hyp);
    int leftWingPos = pos + random(-1,1) - 6;
    int rightWingPos = pos + random(-1,1) + 6;
    
    pixels.clear();
    pixels.setPixelColor(pos,SKIN);
    pixels.setPixelColor((int)leftWingPos, YELLOW);
    pixels.setPixelColor((int)rightWingPos, YELLOW);
    pixels.show();
  }
  
  else{
    pixels.clear();
    pixels.show();
  }
}
 
void animate(struct deviceAngle *angle){
//   switch(direct){
//      case 0:
//        pos = pos;
//        break;
//      case 1:
//        pos -= 1;
//        break;
//      case 2:
//        pos += 1;
//        break;
//      case 3:
//        pos += 6;
//        break;
//      case 4:
//        pos -= 6;
//        break;
//   }
//
//  /* Checks to keep animation rolling over end of LED chain */
//  if(pos >= NUMPIXELS){
//    pos -= NUMPIXELS;
//  }
//  if(pos <= 0){
//    pos += NUMPIXELS;
//  }
//  if(leftWingPos > NUMPIXELS){
//    leftWingPos -= NUMPIXELS;
//  }
//  if(leftWingPos < 0){
//    leftWingPos += NUMPIXELS;
//  }
//  if(rightWingPos > NUMPIXELS){
//    rightWingPos -= NUMPIXELS;
//  }
//  if(rightWingPos < 0){
//    rightWingPos += NUMPIXELS;
//  }
}

void IMUangle(struct deviceAngle *buffer){
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
  buffer->hyp =   (int) (buffer->hyp * 5);
  buffer->theta = (int) (buffer->theta * 10);
  
}

void loop() {
  if(digitalRead(lowBattPin)){
    struct deviceAngle angle = {0,0};
    readImu();
    IMUangle(&angle);
    //animate(&angle);
    draw(&angle);
    delay(FRAMERATE);
  }
  else{
    //shutdown
  }
}

void wake(){
  Serial.println("'Oh?'");
  delay(100);
}
