
// Code designed by Emile, Liliane and Asaph

#include <Servo.h>          //Servo motor library. This is standard library
#include<Wire.h>
#include <math.h>
//gps
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
//gps

const int MPU=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
double pitch,roll;
//===============================================================================
//  Initialization
//===============================================================================

//GPS
// Choose two Arduino pins to use for software serial
int RXPin = 2;
int TXPin = 3;

int GPSBaud = 9600;

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create a software serial port called "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);
//END GPS
//Impact Sensor
int val=0;
//End Impact Sensor

//GSM Module
SoftwareSerial SIM900(7, 8); // RX, TX pins for the SIM900 module
String date;
String latitude;
String longitude;
String driverNumber;
//End GSM Module

// motors
//our L298N control pins

const int LeftMotorForward = 9;
const int LeftMotorBackward = 6;
const int RightMotorForward = 4;
const int RightMotorBackward = 5;

//end motor

void setup(){
  pinMode(12, INPUT); // set pin 12 as input for Impact Sensor
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  // Start the software serial port at the GPS's default baud
  gpsSerial.begin(GPSBaud);
  //SIM900.begin(19200);
  //End GSM 
  SIM900.begin(9600);
  // Give time to your GSM shield log on to network
  delay(20000);
  Serial.print("SIM900 ready...");

  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r"); 
  delay(100);
  // Set module to send SMS data to serial out upon receipt 
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(1000);
  //End GSM
  //motor

  pinMode(RightMotorForward, OUTPUT);
  pinMode(LeftMotorForward, OUTPUT);
  pinMode(LeftMotorBackward, OUTPUT);
  pinMode(RightMotorBackward, OUTPUT);


  //end motor

}
//===============================================================================
//  Main
//===============================================================================
void loop(){

  //motor
moveForward();
  //end motor
Wire.beginTransmission(MPU);
Wire.write(0x3B);
Wire.endTransmission(false);
Wire.requestFrom(MPU,14,true);

int AcXoff,AcYoff,AcZoff,GyXoff,GyYoff,GyZoff;
int temp,toff;
double t,tx,tf;

//Acceleration data correction
AcXoff = -250;
AcYoff = 36;
AcZoff = 1200;

//Temperature correction
toff = -1400;

//Gyro correction
GyXoff = -335;
GyYoff = 250;
GyZoff = 170;

//read accel data and apply correction
AcX=(Wire.read()<<8|Wire.read()) + AcXoff;
AcY=(Wire.read()<<8|Wire.read()) + AcYoff;
AcZ=(Wire.read()<<8|Wire.read()) + AcZoff;

//read temperature data & apply correction
temp=(Wire.read()<<8|Wire.read()) + toff;

//read gyro data & apply correction
GyX=(Wire.read()<<8|Wire.read()) + GyXoff;
GyY=(Wire.read()<<8|Wire.read()) + GyYoff;
GyZ=(Wire.read()<<8|Wire.read()) + GyZoff;

// Calculate and convert temperature
tx=temp;
t = tx/340 + 36.53;     // Formula from data sheet
tf = (t * 9/5) + 32;    // Standard C to F conversion

//get pitch/roll
getAngle(AcX,AcY,AcZ);

//send the data out the serial port
Serial.print("Angle: ");
Serial.print("Pitch = "); Serial.print(pitch);
Serial.print(" | Roll = "); Serial.println(roll);

Serial.print("Temp: ");
Serial.print("Temp(F) = "); Serial.print(tf);
Serial.print(" | Temp(C) = "); Serial.println(t);

Serial.print("Accelerometer: ");
Serial.print("X = "); Serial.print(AcX);
Serial.print(" | Y = "); Serial.print(AcY);
Serial.print(" | Z = "); Serial.println(AcZ);

Serial.print("Gyroscope: ");
Serial.print("X = "); Serial.print(GyX);
Serial.print(" | Y = "); Serial.print(GyY);
Serial.print(" | Z = "); Serial.println(GyZ);
Serial.println(" ");
delay(3333);

// Check for accident
val = digitalRead(12); // read the digital value of pin 12
//Serial.println(val); // print the value to the serial monitor
if((pitch<35 && roll<0) || (pitch<0 && roll>50) || val==1)
{
	 
    Serial.println("\n Accident data sent ");
    sendSMS();
    delay(3000);

   // This sketch displays information every time a new sentence is correctly encoded.
    while (gpsSerial.available() > 0)
      if (gps.encode(gpsSerial.read()))
      get_Coordinates();

      //Send information to NodeMCU
       // Combine data into a comma-separated string
      String data = date + "," + latitude + "," + longitude + "," + String(driverNumber);

      // Send data to NodeMCU
       Serial.println(data);
      //End Send Information to NodeMCU
      delay(2000);
}



}
//===============================================================================
//  COde to Convert accleration data to pitch & roll
//===============================================================================
void getAngle(int Vx,int Vy,int Vz) {
double x = Vx;
double y = Vy;
double z = Vz;
pitch = atan(x/sqrt((y*y) + (z*z)));
roll = atan(y/sqrt((x*x) + (z*z)));
//convert radians into degrees
pitch = pitch * (180.0/3.14);
roll = roll * (180.0/3.14) ;
}
//Method to send SMS
void sendSMS()
{
 

      SIM900.println("AT + CMGS = \"+250738875778\""); // Define Phone number to receive SMS
      delay(100);
      //Message tobe sent
      String dataMessage = "Accident at: https://www.google.com/maps/place/" + latitude + "," + longitude;
      SIM900.print(dataMessage);
      delay(100);
      
      SIM900.println((char)26); 
      delay(100);
      SIM900.println();
      // Give module time to send SMS
      delay(5000); 
      Serial.print("SMS Sent");
}

void get_Coordinates()
{
  if (gps.location.isValid())
  {
   latitude=gps.location.lat();
   longitude=gps.location.lng();
   
  }
}
//Get GPS Data
void displayInfo()
{
  if (gps.location.isValid())
  {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());
  }
  else
  {
    Serial.println("Location: Not Available");
  }
  
  Serial.print("Date: ");
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.println(gps.date.year());
  }
  else
  { Serial.println("Not Available");
  }

  Serial.print("Time: ");
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(".");
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.println(gps.time.centisecond());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.println();
  Serial.println();
  delay(1000);
}
// Move car in front 
void moveForward(){

    digitalWrite(LeftMotorForward, HIGH);
    digitalWrite(RightMotorForward, HIGH);
    digitalWrite(LeftMotorBackward, LOW);
    digitalWrite(RightMotorBackward, LOW);
 
}