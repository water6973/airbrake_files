//code
#define MPU6050_ACCEL_FS_16G 2
#define SEALEVELPRESSURE_HPA (1013.25)//keep?? for mow 
#include <Wire.h>
#include <SPI.h> // for spi
#include <SD.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <I2Cdev.h>
#include <MPU6050.h>

unsigned long flight_time;
unsigned long launchtime;
float Launchalt;
int accel;
int accel_offset_raw =0 ;
int launchstatus = 0;
float altitude;
double altcm;
int servopwm;
const int servoinpwm = 2200;
int servodeg;
int acc_scale = 4096;//imu accel scale
int motor_delay = 2000; // in milliseconds
bool isBurning = false;
float pastAltitude = 0; // in m
float acceleration; // in m/s/s
float velocity; // m/s
float pastVelocity;
float pitch;
float yaw;
float angle_of_attack;
int pastTime;

int16_t ax, ay, az;

Adafruit_BMP3XX bmp;
MPU6050 mpu; 

#define MPU6050_RANGE_250_DEG 1

int deploy = 2; //0 for full, 1 for partial, 2+ for in 
bool isDeployed = false;

File myFile ;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("insert code description");
  Wire.begin();
  
Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  
  mpu.initialize();
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16G);
  pinMode(8,OUTPUT);//ledpin
  pinMode(9,OUTPUT);
  digitalWrite(9,LOW);
  digitalWrite(8,LOW);
  if (!bmp.begin_I2C()) { 
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
 // bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  bmp.readAltitude(SEALEVELPRESSURE_HPA);
  delay(2000); // bjorn does this wait for some kind of pressure sensor init? pls comment magic number stuff// no 
  Launchalt = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.println(Launchalt);
  // also, is this LED just for confirming setup worked? no its a pulse for oscope
  digitalWrite(8,HIGH);
  delay(500);
  digitalWrite(8,LOW); 
   myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  myFile.print("opened");
  
  pastAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-Launchalt;
}
 
 
 
void loop() {
  digitalWrite(8,LOW);//pulse
  
  mpu.getAcceleration(&ax, &ay, &az);
    
  myFile = SD.open("test.txt", FILE_WRITE);

  if (! bmp.performReading()) { 
    Serial.println("Failed to perform reading :(");
    return; 
  }
  
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-Launchalt;
  
  if((float)ax/acc_scale > 2 && launchstatus == 0){
    launchstatus = 1;
    launchtime = micros();
  }

  Serial.println((float)ax/acc_scale);
  Serial.println(launchstatus);

  flight_time = (micros()-launchtime)/1000;
  
  if(launchstatus == 1 && flight_time < 20000){
    acceleration = (float) ax / acc_scale;
    altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-Launchalt;
  
    velocity = (altitude - pastAltitude)/(flight_time - pastTime);
    pastAltitude = altitude;
    acceleration = (velocity - pastVelocity)/(flight_time - pastTime);
    pastVelocity = velocity;
    pastTime = flight_time;
    isBurning = flight_time <= motor_delay;
  
    myFile.print(flight_time);
    myFile.print(", ");

    myFile.print(altitude);
    myFile.print(", ");
    
    myFile.print(velocity);
    myFile.print(", ");

    myFile.print(acceleration);
    myFile.print(", ");
    
    myFile.print(deploy);
    myFile.print(", ");
    
    myFile.print(isBurning);
    myFile.println("");
  
  
    if (flight_time>1999){
      servodeg = 200;
      servopwm = 800;
      digitalWrite(9,HIGH);
      delayMicroseconds(servopwm);
      digitalWrite(9,LOW);
      deploy = 0;
    }
  }
  else if (flight_time >= 20000){
    launchstatus = 0;
    flight_time=0;
    servopwm = servoinpwm;
      if(deploy<30){
      digitalWrite(9,HIGH);
      delayMicroseconds(servopwm);
      digitalWrite(9,LOW);
      deploy ++;
    }
    
    
  }
  myFile.close();
  delay(50);
}
