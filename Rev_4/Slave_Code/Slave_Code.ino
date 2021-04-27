

///// Rev_4, Slave Code /////
///// Added Analog Stick /////
///// Analog Stick is connected to the Slave Arduino /////
///// Analog Stick is read manually without any libraries /////
///// Arduino Slave sends raw Analog Stick Data to Master using i2c bus /////
///// Arduino Master reads and proccess/maps the Analog Stick Data for displaying /////
///// Master Arduino displays the Analog Stick's readings on the OLED display /////
///// OLED display also shows a text graphic that can be moved around using the Analog Stick /////
///// Manual Offsets are applied to Analog Stick's reading to get accurate data /////
///// i2c TX and RX is now done properly with single Bytes for both Master and Slave /////
///// Overall code has also been simplified and more comments have been added /////

///// GitHub: @Hizbi-K /////


#include <Arduino.h>
#include <Wire.h>



byte receiveFromMaster = 0;    
byte sendToMaster = 0;         
byte bufSendToMaster[4] = {0, 0, 0, 0};


int LED_1 = 6;
bool stateLED_1 = LOW;
volatile bool LED_1_Toggle = false;     // Declared as 'volatile' so the variable stays on the RAM for fast access

int LED_2 = 7;
bool stateLED_2 = LOW;
volatile bool LED_2_Toggle = false;     // Declared as 'volatile' so the variable stays on the RAM for fast access


struct analogStickStruct
{
  int buttonPin = 5;
  int xPin = A2;
  int yPin = A1;
  
  volatile int16_t x = 0;               // Declared as 'volatile' so the variable stays on the RAM for fast access
  volatile int16_t y = 0;               // Declared as 'volatile' so the variable stays on the RAM for fast access

  bool butState = LOW;
  bool keyPressed = false;

} analogStick;

volatile bool analogReadOn = false;     // Turning off the Analog Stick reading Fucntion at initial startup


unsigned long previousMillis_1 = 0;     // For Button 1 (Analog Stick's Button)
unsigned long currentMillis = 0;


void analogStickReader ();      // Reads the Analog Stick Data from the analog pins and transmits it to Master using i2c

void slaveRead(int);            // For reading data from Master
void slaveWrite();              // For sending data to Master



////////////////////////////////////////////////////////////////////////////////////



void setup() 
{
  
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);


  pinMode(analogStick.buttonPin, INPUT_PULLUP);
  pinMode(analogStick.xPin, INPUT);
  pinMode(analogStick.yPin, INPUT);


  Wire.begin(9);                // Becoming an i2c Slave with address "9"
  Wire.onReceive(slaveRead);
  Wire.onRequest(slaveWrite);

}



void loop() 
{


  if (analogReadOn == true)     // For reading the Analog Stick Data when requested by the Master Arduino
  {
    analogStickReader();

    analogReadOn = false;
  }

  
  
  if (LED_1_Toggle == true)     // For switching the left LED light when requested by the Master Arduino       
  {
    
    if (stateLED_1 == LOW)
    {
      stateLED_1 = HIGH;
      digitalWrite(LED_1, stateLED_1);
    }
    
    else if (stateLED_1 == HIGH)
    {
      stateLED_1 = LOW;
      digitalWrite(LED_1, stateLED_1);
    }

    
    LED_1_Toggle = false;
    
  }



  if (LED_2_Toggle == true)     // For switching the right LED light when requested by the Master Arduino 
  {
    
    if (stateLED_2 == LOW)
    {
      stateLED_2 = HIGH;
      digitalWrite(LED_2, stateLED_2);
    }
    
    else if (stateLED_2 == HIGH)
    {
      stateLED_2 = LOW;
      digitalWrite(LED_2, stateLED_2);
    } 
 
  
    LED_2_Toggle = false;
    
  }


} 



////////////////////////////////////////////////////////////////////////////////////



void slaveRead (int something)      // For reading the data sent by the Master Arduino 
{                                   // and setting the flag for the appropriate task

  receiveFromMaster = Wire.read();


  switch (receiveFromMaster)
  {

    case 1:
    {
      LED_1_Toggle = true;

      break;
    }
    
    case 2:
    {   
      LED_2_Toggle = true;

      break;
    }

    case 3:
    {
      analogReadOn = true;

      break;
    }
  }


}



////////////////////////////////////////////////////////////////////////////////////



void slaveWrite ()      // For sending the data back when 
{                       // requested by the Master Arduino 
  
  switch(receiveFromMaster)
  {

    case 1:
    {
      sendToMaster = receiveFromMaster;           // Sending back the same data to Master for
      Wire.write(sendToMaster);                   // acknowledging the successfull i2c transmission

      break;
    }
    
    case 2:
    {
      sendToMaster = receiveFromMaster;            // Sending back the same data to Master for
      Wire.write(sendToMaster);                    // acknowledging the successfull i2c transmission

      break;
    }

    case 3:
    {                                                   
      bufSendToMaster[0] = analogStick.x >> 8;      // Filling the i2c Buffer in Bytes with
      bufSendToMaster[1] = analogStick.x;           // X-Axis data from the Analog Stick

      bufSendToMaster[2] = analogStick.y >> 8;      // Filling the i2c Buffer in Bytes with
      bufSendToMaster[3] = analogStick.y;           // Y-Axis data from the Analog Stick

    
      Wire.write(bufSendToMaster, 4); 

      break;  
    }
    
  }
 
}



////////////////////////////////////////////////////////////////////////////////////



void analogStickReader ()     // For reading the Analog Stick Data when requested by the Master Arduino
{
  
  analogStick.x = analogRead(analogStick.xPin);
  analogStick.y = analogRead(analogStick.yPin);

  analogStick.x = analogStick.x - 9;        // Applying the offsets to get the accurate data
  analogStick.y = analogStick.y + 12;       // Applying the offsets to get the accurate data

}



////////////////////////////////////////////////////////////////////////////////////
