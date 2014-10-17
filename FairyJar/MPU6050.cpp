#include "MPU6050.h"
#include <Arduino.h>
#include <Wire.h>

IMUMotion volatile Motion = {0,0,0,0,0,0,0,0};
uint8_t rawIMU[14];

// Reads 14 registers from IMU and normalises them, Puts them in Motion
void readImu(){
  int error;  
  error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &rawIMU, 14); // read all 14 bytes of motion/temp data
  
  Motion.x = (float) ( (int16_t)((rawIMU[0] << 8) | rawIMU[1]) ) / 65536;
  Motion.y = (float) ( (int16_t)((rawIMU[2] << 8) | rawIMU[3]) ) / 65536;
  Motion.z = (float) ( (int16_t)((rawIMU[4] << 8) | rawIMU[5]) ) / 65536;
   
  Motion.temp = ((float) ( (int16_t)((rawIMU[6] << 8) | rawIMU[7]) ) + 12412.0) / 340.0;

  Motion.roll  = (float) ( (int16_t)((rawIMU[8]  << 8) | rawIMU[9] ) ) / 65536;
  Motion.pitch = (float) ( (int16_t)((rawIMU[10] << 8) | rawIMU[11]) ) / 65536;
  Motion.yaw   = (float) ( (int16_t)((rawIMU[12] << 8) | rawIMU[13]) ) / 65536;  
}

// This is a common function to read bytes from an I2C device
int MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);

  n = Wire.endTransmission(false);    // hold the I2C-bus
  if (n != 0)
    return (n);

  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++]=Wire.read();
  }
  if ( i != size)
    return (-11);

  return (0);  // return : no error
}

// This is a common function to write multiple bytes to an I2C device.
// Parameters:
//   start : Start address, use a define for the register
//   pData : A pointer to the data to write.
//   size  : The number of bytes to write.
int MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
    return (-20);

  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
    return (-21);

  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);

  return (0);         // return : no error
}

// An extra function to write a single register.
int MPU6050_write_reg(int reg, uint8_t data)
{
  int error;

  error = MPU6050_write(reg, &data, 1);

  return (error);
}
