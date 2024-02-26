// define constants
#define MPU6050_ACCEL_FS_16G 2
#define MPU6050_RANGE_250_DEG 1 
#define SEALEVELPRESSURE_HPA (1013.25)

// install libraries: since there are thousands of different libraries on Arduino, the only one it has available by default is the standard library. Since we're also using things like a gyroscope, a barometer, and an accelerometer, we need to install their respective libraries to be able to use them.
#include <Wire.h> // wire library (allows communication between Arduino and other equipment)
#include <SPI.h> 
#include <SD.h> // SD card library: allows for the writing of the data to the SD card
#include <Adafruit_Sensor.h> 
#include "Adafruit_BMP3XX.h" // barometer library
#include <I2Cdev.h> 
#include <MPU6050.h> // accelerometer/gyroscope library

unsigned long flight_time; // how long in milliseconds have passed since the rocket detected a launch?
unsigned long launch_time; // how long in milliseconds after the code was deployed did the rocket detect the launch?
float launch_alt; // how high above sea level in meters was the rocket when it was launched?
float acceleration; // in m/s/s
int accel_offset_raw = 0; // sometimes, the accelerometer can be inaccurate by a few m/s/s. this setting offsets the accelerometer's readings to correct them. currently 0 because no such inaccuracy has been detected, but this should be tested next workshop.
int launchstatus = 0; // 0 is 'idle': launch has not yet been detected and the rocket is standing by, 1 is 'active': launch has taken place and the rocket is running recording data, deploying air brakes, etc
float altitude; // in meters
float pitch; // in degrees
float yaw; // in degrees 
float angle_of_attack; // if you take a rocket that is facing vertical and rotate it slightly so it's no longer perpendicular with the ground, you can measure the angle between the centerline of the rocket and a vertical line. this angle is the angle of attack
int servopwm; // extension of air brakes (2200 is unextended, 800 is fully extended, can be set to anything in between). However, not really used in this code, deploy is used instead (which is much less variable, 0 is full, 1 is partial, 2+ is unextended). Will be used in the real adaptive code though.
const int servoinpwm = 2200; // the extension number that will cause the air brakes to be fully unextended
int servodeg; // honestly don't know what this is, but has something to do with the deployment of air brakes. ask bjorn if you want to know
int acc_scale = 4096; // the accelerometer has different sensitivity settings. to get a more sensitive reading, we set the sensitivity of the accelerometer higher (meaning that it gives us larger numbers for precision) but then scale it down to be back in m/s/s. this number is what the raw reading needs to be divided by to get the actual reading.
int burnout = 2000; // in milliseconds, how long the motor will burn for
bool isBurning = false; 
float pastAltitude = 0; // in meters, what was the altitude of the rocket the last time it was recorded? used to calculate change in altitude for velocity
float previousTime = 0; // in milliseconds, what was the time recorded in the last data entry? used to calculate change in time for velocity
float velocity = 0; 
Adafruit_BMP3XX bmp; // create a barometer variable called bmp, we can now manipulate our barometer and get it to record data or calculate altitude
MPU6050 mpu; // do the same for the accelerometer/gyroscope: mpu

// declare variables
int16_t ax, ay, az; // acceleration in the x, y, and z directions (z is vertical and is what we are interested in)
int16_t xx, yy, zz; // not needed, but when we get the data from the gyroscope, it requires us to tell it the variables that each piece of data will be recorded in, even if we don't care about the data it is recording
int16_t gx, gy, gz; // gz is roll, unnecessary, gx and gy are pitch and yaw

const int XGyroOffset = -39; // similar to accelerometer offset. since the gyroscope is on its side in the rocket, it will record pitch and yaw by default as if the rocket is on its side. we calibrate the gyroscope to basically tare the gyroscope like a scale and define its position when the rocket is upright as perfectly level.
const int YGyroOffset = 67; 
const int ZGyroOffset = -20;

int gyro_scale = 131; //same as acceleration scale: mpu6050 gyro scale assuming sensitivity of +/- 250 dps

// calculating angle of attack (there's got to be an easier way, right?)
/*
Math behind calculating angle of attack is kind of complex and also unnecessary to understand since it doesn\'t matter and we might not even use it in the end adaptive code. I\'ll give a brief
explanation though:

First, we isolate the effect of pitch and yaw on the rocket. We draw a line straight through the center of the rocket (the center-line), and then rotate it one way until the angle of the centerline
when compared to vertical is equal to the pitch of the rocket. Then, we do the same with the yaw, but in a perpendicular direction. The end is two lines pointing in perpendicular directions
corresponding to the centerline of the rocket if it was only affected by pitch and yaw.

Second, we extend those lines through the x or y axis to become 2D planes instead of lines. This may be easier to understand in 3D Desmos. The line that forms between the intersection of these
two planes is the centerline of the rocket with both pitch and yaw accounted for. Finding the vector of this line is the hard part, and requires the most math. Basically, we take the normal line
(a vector that is perpendicular to a plane) of both planes and take the cross product of those vectors to get the direction of the final line. Then, we calculate the magnitutde of the vector
using Pythagoran\'s theorem for 3D space.

Finally, we find the angle between this centerline vector and the z-axis vector (which points straight up). This can be done with some simple trigonometry. This is the angle of attack, and we
convert to radians before outputting it.
*/

// creating vector class
struct Vector3 {
    double x, y, z;
};

// Function to calculate the cross product of two vectors
Vector3 crossProduct(const Vector3& v1, const Vector3& v2) {
    Vector3 result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

// Function to calculate the magnitude of a vector
double magnitude(const Vector3& v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Function to calculate the angle of attack given pitch and yaw in degrees
double get_angle_of_attack(double pitch, double yaw) {

    // if pitch and yaw are 90, calculation will not work (probably more cases where this happens to)
    if (pitch == 90 && yaw == 90){
        return 90;
    }

    // Convert angles from degrees to radians
    double rad_pitch = pitch * M_PI / 180.0;
    double rad_yaw = yaw * M_PI / 180.0;

    // Normal vectors of the planes
    Vector3 n1 = {-tan(M_PI/2 - rad_pitch), 0, 1};
    Vector3 n2 = {0, -tan(M_PI/2 - rad_yaw), 1};

    // Direction vector of the line
    Vector3 direction = crossProduct(n1, n2);

    // Calculate the angle of attack (angle between direction vector and z-axis)
    double dot_product = direction.z;  // Dot product with z-axis (0, 0, 1)
    double direction_magnitude = magnitude(direction);
    double angle_of_attack_rad = acos(dot_product / direction_magnitude);

    // Convert angle to degrees
    return angle_of_attack_rad * 180.0 / M_PI;
}

int deploy = 2; // air brake deployment level: 0 for full, 1 for partial, 2+ for in 
// this is somewhat obsolete, will be changing to be more variable (800-2200 instead of 0-2)

File myFile; // this is the file that will be recording data. we set this to myFile so we can manipulate it similar to how we manipulate gyroscope and accelerometer.

void setup() { // the setup part of the code: this happens once when the code is first deployed and never again.
    Serial.begin(115200); // Serial is like the terminal of Arduino, you can use it to print out information while the code is running. The 115200 just means that it is printing to that port, so when you check the monitor, select port 115200 to view the output.
    while (!Serial); // if the code cannot access the Serial monitor (which happens while configuration occurs), it loops until it is finished, then continues.
    Serial.println("insert code description"); 
    Wire.begin(); // Wire allows the Arduino to communicate with all the equipment (gyroscope, accelerometer, barometer), so this just starts it
  
    Serial.print("Initializing SD card..."); 
    if (!SD.begin(4)) { // If the SD card has not started yet (exclamation point indicates "not"), then indicate such and start an infinite loop (this is not good and should be fixed actually) 
        Serial.println("initialization failed!");
        while (1);
    }
    Serial.println("initialization done.");
  
    mpu.initialize(); // initialize the accelerometer/gyroscope
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16G); // set sensitivity of the accelerometer to one of the highest possible, between 0 and 16G since the rocket will never get higher than 16 Gs
    mpu.setFullScaleGyroRange(MPU6050_RANGE_250_DEG); // set sensitivity of gyroscope to be the highest possible as well, between 0 and 250 degrees since the rocket will not exceed 250 degrees of pitch/yaw unless it tumbles
    mpu.setXGyroOffset(XGyroOffset); // set the gyroscope to take into account the calibration offsets that we set at the start
    mpu.setYGyroOffset(YGyroOffset);
    mpu.setZGyroOffset(ZGyroOffset);
    pinMode(8,OUTPUT); // configure these LED lights to activate
    pinMode(9,OUTPUT);
    digitalWrite(9,LOW); // keep them at a low light level
    digitalWrite(8,LOW);
    if (!bmp.begin_I2C()) {   // if the barometer did not start yet, also create indicate such and begin an infinite loop (bro why did bjorn choose to literally brick the code when something doesn't work instead of just retrying)
        Serial.println("Could not find a valid BMP3 sensor, check wiring!");
        while (1);
    }

    // Set up oversampling and filter initialization
    // bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X); // configure barometer settings, what this actually means is unimportant
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);
    bmp.readAltitude(SEALEVELPRESSURE_HPA); // calibrate barometer according to the sea level pressure
    delay(2000); // wait 2 seconds, idk why to be honest, ask bjorn
    launch_alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);  // get the current altitude and set launch altitude to that altitude
    Serial.print("Launch altitude: "); // print out the recorded launch altitude to the monitor (NOTE: this is different from recording it to the data file itself) 
    Serial.println(launch_alt); 

    // pulses LED light to confirm that gyroscope, accelerometer, and barometer setup correctly
    digitalWrite(8,HIGH); // turn on light bright
    delay(500); // wait half a second
    digitalWrite(8,LOW); // make the light low again
    myFile = SD.open("test.txt", FILE_WRITE); // open a file text.txt on the SD card and set that file to be myFile (the file that we are manipulating) and tell the computer we intend to write on it. if text.txt does not exist on the SD card, it makes a new one
    // if the file opened okay, write to it:
    if (myFile) {
        Serial.print("Writing to test.txt...");
    } else {
        // if the file didn't open, print an error. hey, there's no infinite loop here, incredible
        Serial.println("error opening test.txt");
    }
    myFile.print("opened"); // print to the file that it has been opened, allowing us to look through the data and clean it better by providing partitions for different code activations
        myFile.close();
    pastAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launch_alt; // calculate pastAltitude (which is effectively just launch_alt) so that there is a number associated with pastAltitude during the first velocity calculations during launch
}
 
 
 
void loop() { // this code loops forever while the code is running
    digitalWrite(8,LOW); // turn on the light to a low level (since this is in the loop, it's basically a pulse)
    
    mpu.getAcceleration(&ax, &ay, &az); // get acceleration values from the accelerometer (ax is all we care about, vertical acceleration)
    mpu.getMotion6(&xx, &yy, &zz, &gx, &gy, &gz); // get gyroscopic values from the gyroscope (gx and gy is all we care about, pitch and yaw)

    acceleration = (((float) ax/acc_scale) - 1) * 9.80665 // convert acceleration from Gs to m/s/s
        
    myFile = SD.open("test.txt", FILE_WRITE); // open the file again (since we close it at the end of each loop to save it, we need to reopen it at the start of each loop)

    if (! bmp.performReading()) { // if barometer failed to get a reading, then say that in the monitor (but continue with the rest of the loop anyways)
        Serial.println("Failed to perform reading :(");
        return; 
    }
     
    altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launch_alt; // calculate current altitude by getting the height above sea level and subtracting it by the measured height above sea level when launch was detected
    
    // detects launch by checking if acceleration is higher than 2 m/s/s and the rocket is in idle mode
    if(acceleration > 2 && launchstatus == 0){
        launchstatus = 1; // set rocket to active mode
        launch_time = micros(); // record what time the launch took place
    }

    // print out data on acceleration and launch status (for debugging to see if accelerometer is calibrated right and whether it is detecting launch correctly)
    Serial.print("Vertical Acceleration (m/s/s): "); 
    Serial.println(acceleration);
    Serial.print("Launch status: ");
    Serial.println(launchstatus);

    // if launch is detected and rocket has been flying for less than 10 seconds
    if(launchstatus == 1 && flight_time < 10000){
        // get all relevant launch variables
        flight_time = (micros()-launch_time)/1000; // get current flight time by subtracting current time from the time launch was detected.
        altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)-launch_alt; // get altitude by subtracting current altitude from altitude by launch
        velocity = (altitude - pastAltitude)/(micros()-previousTime); // calculate velocity by getting change in altitude over change in time. this method is prone to fluctuations, but it's the best we got for now.
        previousTime = micros(); // set previousTime to be current time so that the next entry uses this entry's time for change in time calculations
        pastAltitude = altitude; // same thing for altitude
        pitch = gx/gyro_scale; // get and scale pitch from gyroscope data
        yaw = gy/gyro_scale; // get and scale yaw from gyroscope data
        angle_of_attack = get_angle_of_attack(pitch, yaw); // calculate angle of attack using pitch and yaw
        isBurning = flight_time <= burnout; // if less time has passed since launch than the time the motor will burn for, then the motor is still burning. otherwise, it isn't.
Serial.println(flight_time);
        // record out relevant launch variables to the data file (text.txt). add the commas so that it can be easily converted to a .csv file for graphing, cleaning, and analyzing
        myFile.print(flight_time);
        myFile.print(", ");
        
        myFile.print(velocity);
        myFile.print(", ");
        
        myFile.print(altitude);
        myFile.print(", ");
        
        myFile.print(pitch);
        myFile.print(", ");
        
        myFile.print(yaw);
        myFile.print(", ");
        
        myFile.print(deploy);
        myFile.print(", ");
        
        myFile.print(isBurning);
        myFile.print(", ");
        
        myFile.print(acceleration);
        myFile.print(", ");
        
        myFile.print(angle_of_attack);
        myFile.println();
        
        // if rocket has been flying for more than two seconds
        if (flight_time>1999){
            // deploy air brakes
            servodeg = 200;
            servopwm = 800;
            digitalWrite(9,HIGH); // pulse the lights
            delayMicroseconds(servopwm);
            digitalWrite(9,LOW);
            deploy = 0; // set deploy to indicate the brakes are fully extended
        }  
    }

    // if the rocket has been flying for more than 10 seconds
    else if (flight_time >= 10000){
        // set rocket to idle
        launchstatus = 0;
        flight_time = 0; // reset flight time
        servopwm = servoinpwm;
        // undeploy air brakes
        if(deploy<30){ // if deploy is less than thirty
            digitalWrite(9,HIGH); // pulse the lights no this is the actual signal to the servo
            delayMicroseconds(servopwm);
            digitalWrite(9,LOW);
            deploy ++; // increase the deploy variable to indicate that the air brakes are no longer fully extended
        }
    }
    // close data file to save changes
    myFile.close();
}