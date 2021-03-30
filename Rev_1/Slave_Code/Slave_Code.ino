

#include <Arduino.h>
#include <Wire.h>



byte receiveFromMaster = 11;    // Dummy Values
byte sendToMaster = 11;         // Dummy Values


int LED_1 = 6;
bool stateLED_1 = LOW;
byte LED_1_Check = 0;
volatile bool LED_1_Toggle = false;

int LED_2 = 7;
bool stateLED_2 = LOW;
byte LED_2_Check = 1;
volatile bool LED_2_Toggle = false;


void slaveRead(byte);
void slaveWrite();



////////////////////////////////////////////////////////////////////////////////////



void setup() 
{

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);


  Wire.begin(9);
  Wire.onReceive(slaveRead);
  Wire.onRequest(slaveWrite);

}



void loop() 
{

  
  if ((LED_1_Toggle == true) && (receiveFromMaster == LED_1_Check))
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
    LED_2_Toggle = false;
    
  }



  if ((LED_2_Toggle == true) && (receiveFromMaster == LED_2_Check))
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
 
  
    LED_1_Toggle = false;
    LED_2_Toggle = false;
    
  }





} 



////////////////////////////////////////////////////////////////////////////////////



void slaveRead (int something)
{

  receiveFromMaster = Wire.read();
  LED_1_Toggle = true;
  LED_2_Toggle = true;

}



////////////////////////////////////////////////////////////////////////////////////



void slaveWrite ()
{
  
  sendToMaster = receiveFromMaster;
  Wire.write(sendToMaster);
   
}



////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////////
