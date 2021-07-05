#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, 0x3C, -1);

#include <PulseSensorPlayground.h> 
PulseSensorPlayground pulseSensor;
const int PulseInput = A1; // input pin for the pulse sensor
int Threshold = 550;
int myBPM;

unsigned long time;
int seconds = 0;
int minutes = 0;
int hours = 0;   

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

const byte buttonPin = 3;

bool stepCheckA=false;
bool stepCheckB=false;
int STEPS =0;

void setup() {
  
  pulseSensor.analogInput(PulseInput);
  pulseSensor.setThreshold(Threshold);

  attachInterrupt(digitalPinToInterrupt(buttonPin), reset, FALLING);

  Wire.begin();
  setupMPU();
}

void loop() {
  time = millis();
   
  myBPM = pulseSensor.getBeatsPerMinute(); // Calls function on the pulseSensor object that returns BPM.
  
  recordAccel();
  recordGyro();
  stepA();
  stepB();
  if(stepCheckA && stepCheckB){//if both the stepcheck and passed
    STEPS += 1;
  }
  display_stuff();
}

void reset() {
  STEPS = 0;// resets the no. of steps when button is pressed.
}

void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU 
  Wire.write(0x6B); //Accessing the register 6B 
  Wire.write(0b00000000); //Setting SLEEP register to 0.
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B  
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C  
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccel() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData(){
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;
}

void recordGyro() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}


void stepA(){
  stepCheckA = false; 
  int a = 0;
  int flag=0;
  float rotXA[100]={0}; // array storing 100 rot values
  float rotYA[100]={0};
  float rotZA[100]={0};
  float rotXAvg=0;     // avg rot value
  float rotYAvg=0;
  float rotZAvg=0;
  
  rotXAvg += rotX/100; // adding new rot values to the average
  rotYAvg += rotY/100;
  rotZAvg += rotZ/100;
    
  if(a>100){
    rotXAvg -= rotXA[0]/100;// removing older rotation values from the average
    rotYAvg -= rotYA[0]/100;
    rotZAvg -= rotZA[0]/100;
    for(int b=99; b>0; b--){
      rotXA[b]=rotXA[b-1];// shifting the rot values by one space in the array
      rotXA[b]=rotXA[b-1];
      rotXA[b]=rotXA[b-1];  
    }
  }
  else{a++;}
     
  rotXA[a] = rotX;// adding the new rotation values to the array 
  delay(1);
  rotYA[a] = rotY;
  delay(1);
  rotZA[a] = rotZ;
  delay(1);

  if(rotX>rotXAvg && rotY>rotYAvg && flag==1){
    stepCheckA=true;
    flag = 0; 
  }
  if(rotX<rotXAvg && rotY<rotYAvg && flag==0){
    flag = 1;
  }
}

void stepB(){
    
   stepCheckB = false;  
   int a = 0;
   int flag = 0;
   int jerk = 2;
   float gForceXA[100]={0};
   float gForceYA[100]={0};
   float gForceZA[100]={0};
   float gForceXAvg=0;
   float gForceYAvg=0;
   float gForceZAvg=0;
   float gForceAvg=0;
   float gForceTot=0;
   
    gForceXAvg -= gForceXA[0]/100;
    gForceYAvg -= gForceYA[0]/100;
    gForceZAvg -= gForceZA[0]/100;
    
    if(a>100){
      for(int b=99; b>0; b--){
        gForceXA[b]=gForceXA[b-1];
        gForceXA[b]=gForceXA[b-1];
        gForceXA[b]=gForceXA[b-1];  
      }}
    else{a++;}
    gForceXA[a] = gForceX;
    delay(1);
    gForceYA[a] = gForceY;
    delay(1);
    gForceZA[a] = gForceZ;
    delay(1);

    gForceXAvg += gForceX/100;
    gForceYAvg += gForceY/100;
    gForceZAvg += gForceZ/100;     


    gForceTot = sqrt(pow(gForceX,2)+pow(gForceY,2)+pow(gForceZ,2));
    gForceAvg = sqrt(pow(gForceXAvg,2)+pow(gForceYAvg,2)+pow(gForceZAvg,2));
    if((gForceTot - gForceAvg > jerk) && flag==1) {
      stepCheckB = true;
      flag = 0;      
    }
    if((gForceTot - gForceAvg < jerk) && flag==0) {
      flag = 1;
    }
}

void timer() {
  seconds = millis() % 60;
  minutes = (millis() /60)%60;
  hours = (millis() /3600)%24;
}

void display_stuff() {

display.clearDisplay();
display.setTextColor(WHITE);

//Time
timer();
display.setCursor(0,0);
display.setTextSize(1);
display.print("Time->");
display.print(hours);
display.print(":");
display.print(minutes);
display.print(":");
display.print(seconds);

//STEPS
display.setCursor(0,14);
display.setTextSize(1);
display.print("Steps->");
display.print(STEPS);

//BPM
display.setCursor(0,28);
display.setTextSize(1);
display.print("BMP->");
display.print(myBPM);

display.display();
delay(2000);
}
