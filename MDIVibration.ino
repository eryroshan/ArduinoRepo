#include "Wire.h"
#include <MPU6050_light.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>


const byte rxPin = 2;
const byte txPin = 3;
SoftwareSerial SUNAserial(rxPin, txPin);

MPU6050 mpu(Wire);
unsigned long timer = 0;
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";
const int buttonPin = 4;
int buttonState = 0;
int EEPROMADDX = 2;
int EEPROMADDY = 4;
int EEPROMADDZ = 6;
int EEPROMADDLevel = 8;

float Xoffset = 0;
float Yoffset = 0;
float Zoffset = 0;

unsigned long startTime = 0;

class Axis{
  private:
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float angle = 0.0;
  public:
  Axis(float x, float y, float z, float angle){
    this->x = x - Xoffset;
    this->y = y - Yoffset;
    this->z = z - Zoffset;
    this->angle = angle;
  }

  float getX(){
    return this->x;
  }

    float getY(){
    return this->y;
  }

    float getZ(){
    return this->z;
  }

   String convertedData(){
     return String(this->x) +','+ String(this->y)+','+ String(this->z)+','+ String(angle);
   }
};

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupRadio();
  //read the offset for all 3 axises
  readOffset();
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
 
  pinMode(buttonPin, INPUT);
 
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ 
    //FIXME: print ERR on the screen 
    Serial.println("Could not connect to the sensor !!!");
  } 
  
  delay(100);
  //in case it needs to be installed upside down --> mpu.upsideDownMounting = true;
  // calibrate 
  //FIXME: print SUCC on the screen
  Serial.println("Done!\n");


}

void loop() {

  unsigned long currTime = millis();
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH){
    calibrate();
  }
  if(currTime - startTime > 2){
    sendData();
    startTime = currTime;
  }

 
}

void setupRadio(){
  radio.begin();
  radio.setChannel(0x76);
 // radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.setPALevel(RF24_PA_MAX);
  const uint64_t pipe = 0xE8E8F0F0E1LL;
  //radio.openReadingPipe(1, pipe);

  radio.enableDynamicPayloads();
  radio.powerUp();
  radio.stopListening();
}

void sendData(){
 if((millis()-timer)>2){
    mpu.update();
    Axis axis(mpu.getAccX(), mpu.getAccY(), mpu.getAccZ(), mpu.getAccAngleX());
    Serial.println(axis.convertedData());
    String data = axis.convertedData();
    radio.write(&data, sizeof(data));
    timer = millis();
  }
}

void calibrate(){
    Serial.println(F("Calculating offsets, do not move MPU6050"));
    float XOffset = 0;
    float YOffset = 0;
    float ZOffset = 0;
    float LevelOffset = 0;

    mpu.calcOffsets();
    delay(100);

    for(int i=0; i<1000; i++){
      XOffset += mpu.getAccX();
      YOffset += mpu.getAccY();
      ZOffset += mpu.getAccZ();
      LevelOffset += mpu.getAccAngleX();
      delay(2);
    }
    EEPROM.update(EEPROMADDX, XOffset/1000.0);
    EEPROM.update(EEPROMADDY, YOffset/1000.0);
    EEPROM.update(EEPROMADDZ, ZOffset/1000.0);
    EEPROM.update(EEPROMADDLevel, LevelOffset/1000.0);
    
    Serial.println("Done!\n");
    delay(100);
}

void readOffset(){
    Xoffset = EEPROM.read(EEPROMADDX);
    Yoffset = EEPROM.read(EEPROMADDX);
    Zoffset = EEPROM.read(EEPROMADDX);
}



  
