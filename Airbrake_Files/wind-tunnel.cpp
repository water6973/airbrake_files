// The code may break if it runs for longer than 71.6 minutes due to micros() overflowing.
// NOTE: REMEMBER TO UPDATE MASS EVERY LAUNCH!!!
// MPU6050 - Version: 1.3.0
#include <MPU6050.h>
// define constants
#define MPU6050_ACCEL_FS_16G 2
#define SEALEVELPRESSURE_HPA (1013.25)
#define DEPLOYED 800
#define UNDEPLOYED 2200
// install libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <I2Cdev.h>

int flightTime = 0;
float altitude = 0;

File myFile;
Adafruit_BMP3XX bmp;
MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Now starting: Wind Tunnel Test");
    Wire.begin();
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("Initialization failed! (Hint: Put in an SD card!)");
        while (1);
    }
    Serial.println("SD card initialization successful! (1/4)\n");
    Serial.println("Initializing accelerometer/gyroscope...");
    mpu.initialize();
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16G);
    pinMode(8,OUTPUT);
    pinMode(9,OUTPUT);
    digitalWrite(9,LOW);
    digitalWrite(8,LOW);
    Serial.println("Accelerometer/Gyroscope initialization successful! (2/4)\n");
    Serial.println("Initializing barometer...");
    if (!bmp.begin_I2C()) {
        Serial.println("Could not find a valid BMP3 sensor, check wiring!");
        while (1);
    }
    // Set up oversampling and filter initialization
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);
    bmp.readAltitude(SEALEVELPRESSURE_HPA);
    digitalWrite(8,HIGH);
    delay(500);
    digitalWrite(8,LOW);
    Serial.println("Barometer initialization successful! (3/4)");
    Serial.print("Launch altitude: ");
    Serial.print(launchAlt);
    Serial.println("m\n");
    Serial.println("Testing data collection...");
    myFile = SD.open("data.txt", FILE_WRITE);
    
    // if the file opened okay, write to it:
    if (myFile) {
        Serial.println("Successfully writing to data.txt! (4/4)\nInitialization successful!\n");
    } else {
        // if the file didn't open, print an error.
        Serial.println("Error opening data.txt!");
    }
    myFile.println("Opened!");
    myFile.close();
    altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launchAlt;
    Serial.println("Beginning loop...");
}

void loop() {
    digitalWrite(8,LOW);
    myFile = SD.open("data.txt", FILE_WRITE);
    altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launchAlt;
    
    // print out data on acceleration and launch status
    Serial.print("Flight Time (ms): ");
    Serial.println(flightTime);
    Serial.print("Altitude (m): ");
    Serial.println(altitude);
    Serial.print("\n");

    flightTime = micros()/1000;
    altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launchAlt;

    myFile.print(flightTime);
    myFile.print(", ");
    myFile.print(altitude);

    if ((flightTime / 2500) % 2 == 0){
        digitalWrite(9,HIGH);
        delayMicroseconds(UNDEPLOYED);
        digitalWrite(9,LOW);
    } else {
        digitalWrite(9,HIGH);   
        delayMicroseconds(DEPLOYED);
        digitalWrite(9,LOW);
    }
    myFile.close();
}