// The code may break if it runs for longer than 71.6 minutes due to micros() overflowing.


// MPU6050 - Version: 1.3.0
#include <MPU6050.h>

// define constants
#define MPU6050_ACCEL_FS_16G 2
#define SEALEVELPRESSURE_HPA (1013.25)
#define DEPLOYED 800
#define UNDEPLOYED 2200

// install libraries:a
#include <Wire.h> 
#include <SPI.h> 
#include <SD.h> 
#include <Adafruit_Sensor.h> 
#include "Adafruit_BMP3XX.h" 
#include <I2Cdev.h> 
#include <MPU6050.h> 

unsigned long int launchTime;
unsigned long int flightTime; // how long in milliseconds have passed since the rocket detected a launch?
int lastTime = 0; // in milliseconds, what was the flight time recorded in the last data entry?
float launchAlt; // how high above sea level in meters was the rocket when it was launched?
float altitude; // in meters above launch altitude
float lastAltitude = 0; // in meters
float velocity = 0; // in m/s
const int accScale = 4096;

const int burnout = 1500; // in milliseconds, how long the motor will burn for
bool isBurning = false;
bool isLaunched = false;

int numEntries = 1; // number of entries to be used in averaging projected apogee
int entry = 0; // current entry to be updated (oldest entry)
int meanProjectedApogee;

const float mass = 0.613; // in kg, after burnout (so not including 33g of F-51 propellant)
const float k = 0.0016;
const float g = 9.8;
const int sliceLength = 5; 

const bool debugMode = false;

Adafruit_BMP3XX bmp; 
MPU6050 mpu; 

int16_t ax, ay, az; // acceleration in the x, y, and z directions
bool isDeployed = false; // air brake deployment status

// create flight_data handler to average projected apogees
struct FlightData {
  int raw_projected_apogee;
};

float get_projected_apogee(FlightData data[], int numEntries){
    float total = 0; 
    for (int i = 0; i < numEntries; i++) {
        total += data[i].raw_projected_apogee;
    }
    return total / numEntries;
}

FlightData flightData[sliceLength];
File myFile; 

void setup() { 
    Serial.begin(115200);
    while (!Serial); 
    Serial.println("Now initializing: AC-DC V4.1\n");
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
    delay(2000); 
    launchAlt = bmp.readAltitude(SEALEVELPRESSURE_HPA);  
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
    lastAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launchAlt;
    
    Serial.println("Beginning loop...\n\n");  
}
 
 
 
void loop() { 
    digitalWrite(8,LOW); 
    
    mpu.getAcceleration(&ax, &ay, &az); // get acceleration values from the accelerometer
       
    myFile = SD.open("data.txt", FILE_WRITE); 

    if (! bmp.performReading()) { 
        Serial.println("Barometer failed to perform reading :(");
        return; 
    }
     
    altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launchAlt;
    
    // detects launch by checking if acceleration is higher than 2 m/s/s and the rocket is in idle mode
    if((float)ax/accScale > 2 && !isLaunched){
        isLaunched = true;
        launchTime = micros();
    }

    // print out data on acceleration and launch status 
    Serial.print("Vertical Acceleration (m/s/s): "); 
    Serial.println((float)ax/accScale);
    Serial.print("Launch status: ");
    Serial.println(isLaunched);

    // if launch is detected and rocket has been flying for less than 20 seconds
    if((isLaunched && flightTime <= 20000) or debugMode){
        // get all relevant launch variables
        flightTime = (micros()-launchTime)/1000; 
        altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launchAlt; 
        velocity = (altitude - lastAltitude)/(flightTime-lastTime) / 1000; 
        lastTime = flightTime; 
        lastAltitude = altitude; 
        isBurning = flightTime <= burnout; 
        flightData[entry].raw_projected_apogee = altitude + (mass / (2*k)) * log((k * pow(velocity, 2)) / (mass * 9.8) + 1);

        // update code
        meanProjectedApogee = get_projected_apogee(flightData, numEntries);
        if (meanProjectedApogee > 250){
            isDeployed = true;
        } else { isDeployed = false; }
        
        // record out relevant launch variables to the data file (data.txt)
        myFile.print(flightTime);
        myFile.print(", ");
        
        myFile.print(altitude);
        myFile.print(", ");
        
        myFile.print(velocity);
        myFile.print(", ");
        
        myFile.print(isDeployed);
        myFile.print(", ");
        
        myFile.print(isBurning);
        myFile.print(", ");
        
        myFile.print(meanProjectedApogee);
        myFile.println();
        
        entry = (entry + 1) % sliceLength;
        numEntries = min(numEntries + 1, sliceLength);
    }
    
    // if rocket has been flying for more than two seconds
    if (flightTime >= 2000 && flightTime < 10000){
      if (isDeployed){
        // deploy air brakes if adaptive code says they should be deployed
        digitalWrite(9,HIGH); 
        delayMicroseconds(DEPLOYED);
        digitalWrite(9,LOW); 
      } else { 
        digitalWrite(9,HIGH);
        delayMicroseconds(UNDEPLOYED);
        digitalWrite(9,LOW);
      }
    }
    
    // if rocket has been flying for more than 10 seconds
    if (flightTime >= 10000){
        isDeployed = false;
        digitalWrite(9,HIGH); 
        delayMicroseconds(UNDEPLOYED);
        digitalWrite(9,LOW);
    }
    
    // if the rocket has been flying for more than 20 seconds
    if (flightTime >= 20000){
        // set rocket to idle
        isLaunched = false;
        flightTime = 0; // reset flight time
    }
    
    // close data file to save changes
    myFile.close();
}