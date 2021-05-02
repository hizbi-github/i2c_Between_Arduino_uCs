

///// Rev_5, Master Code /////
///// Added Analog Stick Button Functionality /////
///// The Analog Stick Button's state is communicated from the Slave to Master using i2c Bus.
///// Pushing the Analog Stick Button enables the "Display Page Swipe" mode /////
///// The Analog Stick can then be moved/swiped left or right to change the Display Pages /////
///// A certian threshold is applied to the Analog Stick's swiping motion /////
///// Also implemeted the i2c Transmission in a Function to reduce repetative code /////
///// Overall code has also been simplified and more comments have been added /////

///// GitHub: @Hizbi-K /////


#include <Arduino.h>
#include <Wire.h>



byte receiveFromMaster = 0;    
byte sendToMaster = 0;         
byte bufSendToMaster[5] = {0, 0, 0, 0, 0};


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
  volatile byte sendButState = 0;

  bool butState = LOW;
  bool keyPressed = false;

} analogStick;

volatile bool analogReadOn = false;     // Turning off the Analog Stick reading Fucntion at initial startup


unsigned long previousMillis_1 = 0;     // For Button 1 (Analog Stick's Button)
unsigned long currentMillis = 0;
unsigned long debounceInterval = 50;


void pressAnalogStick_Btn ();
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

  currentMillis = millis();


  pressAnalogStick_Btn();

  
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
      bufSendToMaster[0] = analogStick.sendButState;    // Sending the Analog Stick's Button State to Master
      
      bufSendToMaster[1] = analogStick.x >> 8;      // Filling the i2c Buffer in Bytes with
      bufSendToMaster[2] = analogStick.x;           // X-Axis data from the Analog Stick

      bufSendToMaster[3] = analogStick.y >> 8;      // Filling the i2c Buffer in Bytes with
      bufSendToMaster[4] = analogStick.y;           // Y-Axis data from the Analog Stick

    
      Wire.write(bufSendToMaster, 5); 

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



void pressAnalogStick_Btn ()      // Using the Analog Stick's Push Button
{
    
    
  if (analogStick.butState != digitalRead(analogStick.buttonPin))
  {  
    
    if (analogStick.butState == HIGH)
    {
      analogStick.butState = digitalRead(analogStick.buttonPin);
      analogStick.keyPressed = true;

      previousMillis_1 = currentMillis;
    }
    else if (analogStick.butState == LOW)
    {
      analogStick.butState = HIGH;
      analogStick.keyPressed = false;
    }

  }
  
  
  if ((analogStick.butState == LOW) && (analogStick.keyPressed == true) && ((currentMillis - previousMillis_1) > debounceInterval))
  {

    analogStick.sendButState = analogStick.sendButState + 1;

    if (analogStick.sendButState > 1)
    {
      analogStick.sendButState = 0;
    }
            
    
    if (digitalRead(analogStick.buttonPin) == LOW)      // No toggling until the finger has been lifted off
    {
      analogStick.keyPressed =  false;
      analogStick.butState = LOW;
    }
    else if (digitalRead(analogStick.buttonPin) == HIGH)
    {
      analogStick.keyPressed =  true;
      analogStick.butState = HIGH;
    }

  } 

}



////////////////////////////////////////////////////////////////////////////////////
