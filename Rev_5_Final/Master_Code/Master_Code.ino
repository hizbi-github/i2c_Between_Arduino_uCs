

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
#include <U8glib.h>



U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 



byte sendToSlave_1 = 0;          // For accessing LED 1
byte receiveFromSlave_1 = 0;

byte sendToSlave_2 = 0;          // For accessing LED 2
byte receiveFromSlave_2 = 0;

byte sendToSlave_3 = 0;          // For accessing Analog Stick on Slave Arduino using i2c


int BUT_1 = A0;
bool butState_1 = LOW;
bool keyPressed_1 = false;
bool stateLED_1 = LOW;

int BUT_2 = A1;
bool butState_2 = HIGH;
bool keyPressed_2 = false;
bool stateLED_2 = LOW;


int encoder_Clk = 2;
volatile bool current_Clk_State = HIGH;

int encoder_Data = 3;
volatile bool current_Data_State = HIGH;

#define INTERRUPT 0       // Setting up Digital Pin 2 for Interrupts from Rotary Encoder

volatile bool encoderStatus = HIGH;
volatile bool encoderBusy = false;

int encoder_BUT = 4;
bool butState_3 = LOW;
bool keyPressed_3 = false;

volatile int upCounter = 0;
volatile int downCounter = 100;


struct rawAccelStruct
{
  int16_t x = 0;
  int16_t y = 0;
  int16_t z = 0;

} rawAccel;

struct cookedAccelStruct
{
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

} cookedAccel;


struct rawGyroStruct
{
  int16_t x = 0;
  int16_t y = 0;
  int16_t z = 0;

} rawGyro;

struct cookedGyroStruct
{
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

} cookedGyro;

struct offsetGyroStruct
{
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

} offsetGyro;


struct analogStickStruct
{ 
  volatile int16_t x = 0;
  volatile int16_t y = 0;

  bool butState = LOW;
  volatile byte receiveButState = 0;

} analogStick;


int switchDisplay = 1;                // Display the first page at startup


String displayLED_1 = "OFF";
String displayLED_2 = "OFF";


unsigned long currentMillis = 0;
unsigned long previousMillis_1 = 0;     // For Button 1
unsigned long previousMillis_2 = 0;     // For Button 2
unsigned long previousMillis_3 = 0;     // For Button 3 (Rotary Encoder's Button)
unsigned long previousMillis_4 = 0;     // For LED (Right)
unsigned long previousMillis_5 = 0;     // For LED (Left)
unsigned long previousMillis_6 = 0;     // Common for calling many different Functions
unsigned long previousMillis_7 = 0;     // For Analog Stick Reception
unsigned long previousMillis_8 = 0;     // For Analog Stick Swiping

unsigned long debounceInterval = 50;
unsigned long encoderDebounce = 50;
unsigned long analogStickSwipeInterval = 250;
unsigned long timeOffMPU = 125;


void pressBUT_1 ();
void pressBUT_2 ();
void pressEncoderBUT ();

void encoderISR ();         // Reacting quickly to Rotary Encoder's Change in Direction usign Interrupts

void init_Accel_Gyro ();    // Initializing MPU 6050 with Custom Settings
void readingAccel();
void readingGyro();

void leftLED_Comm ();
void rightLED_Comm ();

void drawPage_1 ();
void drawPage_2 ();
void drawPage_3 ();
void drawPage_4 ();
void drawPage_5 ();
void drawPage_6 ();

void i2c_Tx_Arduinos (byte Address, byte DataToBeSent, byte Rx_DataSizeinBytes);      // For starting basic Transmission between Master and Slave

void analogStickReception ();     // For receiving the Analog Stick's data from Slave Arduino
void analogStickSwipe ();         // For switching the display menu by swiping the Analog Stick

long rangeMapper (double value, double fromMin, double fromMax, double toMin, double toMax);     // For Mapping two different data ranges



/////////////////////////////////////////////////////////////////////////



void setup() 
{

    if ( u8g.getMode() == U8G_MODE_R3G3B2 )           // Assigning default color values.
    {
      u8g.setColorIndex(255);     // white
    }
    else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) 
    {
      u8g.setColorIndex(3);         // max intensity
    }
    else if ( u8g.getMode() == U8G_MODE_BW )          // I think we are using this, since our display
    {                                                 // is Black & Blue, so Index(1) means to turn on
      u8g.setColorIndex(1);         // pixel on       // the blue pixels (or white pixels in B&W diplays).
    }
    else if ( u8g.getMode() == U8G_MODE_HICOLOR ) 
    {
      u8g.setHiColorByRGB(255,255,255);
    }

  
    pinMode(BUT_1, INPUT_PULLUP);
    pinMode(BUT_2, INPUT_PULLUP);

    pinMode(encoder_Clk, INPUT_PULLUP);
    pinMode(encoder_Data, INPUT_PULLUP);
    pinMode(encoder_BUT, INPUT_PULLUP);


    attachInterrupt(INTERRUPT, encoderISR, CHANGE);


    init_Accel_Gyro();


    Wire.begin(); 


    previousMillis_6 = millis();      // Initializing the timer for the common always-running Functions
    previousMillis_7 = millis();      // Initializing the timer for always-running Analog Stick Reception Function
    previousMillis_8 = millis();      // Initializing the timer for always-running Analog Stick Swipe Function
  
}



void loop() 
{

  currentMillis = millis();


  pressBUT_1();

  pressBUT_2();

  pressEncoderBUT();

  

  if ((currentMillis - previousMillis_7) > debounceInterval)      // Activating the Analog Stick
  {                                                               // reception from Slave Arduino
    analogStickReception();

    previousMillis_7 = currentMillis;
  }


  if ((currentMillis - previousMillis_8) > analogStickSwipeInterval)
  {
    analogStickSwipe();

    previousMillis_8 = currentMillis;
  }


  if (switchDisplay == 3)     // Activating the fucntion for Accel. data 
  {
    if ((currentMillis - previousMillis_6) > timeOffMPU)
    {
      readingAccel();

      previousMillis_6 = currentMillis;
    }
    
  }


  if (switchDisplay == 4)     // Activating the fucntion for Gyro. data 
  {
    if ((currentMillis - previousMillis_6) > timeOffMPU)
    {
      readingGyro();

      previousMillis_6 = currentMillis;
    }
    
  }
  

  if (switchDisplay == 2)             // Only using the encoder if the display
  {                                   // has been switched to the Counter page
    
    if (encoderBusy == true)
    {
      
      if ((encoderStatus == HIGH) && ((currentMillis - previousMillis_4) > encoderDebounce))          // Up counter for clockwise rotation
      {
        upCounter = upCounter + 1;
        rightLED_Comm();         // Flashing the appropriate LED to show the Encoder's direction

        if (displayLED_1 == "ON")
        {
          leftLED_Comm();        // Turning the other LED OFF
        }

        if (upCounter == 101)     // Resetting the counter
        {
          upCounter = 0;
        }

      }


      else if ((encoderStatus == LOW) && ((currentMillis - previousMillis_5) > encoderDebounce))      // Down counter for anit-clockwise rotation
      {
        downCounter = downCounter - 1;
        leftLED_Comm();          // Flashing the appropriate LED to show the Encoder's direction

        if (displayLED_2 == "ON")
        {
          rightLED_Comm();       // Turning the other LED OFF
        }


        if (downCounter < 0)      // Resetting the counter
        {
          downCounter = 100;
        }

      }


      encoderBusy = false;             // Allowing Arduino to read the Rotary Encoder again

    }

  }



  switch(switchDisplay)       // Switching between the LED Status and Counter pages
  {

    case 1:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_1();         // LED Status page
      
      } 
      while (u8g.nextPage());

      break;
    }

    case 2:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_2();         // Counter page
      
      } 
      while (u8g.nextPage());

      break;   
    }

    case 3:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_3();         // 6050 MPU Accel page
      
      } 
      while (u8g.nextPage());

      break;   
    }

    case 4:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_4();         // 6050 MPU Gyro page
      
      } 
      while (u8g.nextPage());

      break;   
    }

    case 5:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_5();         // Analog Stick's Axis Data page
      
      } 
      while (u8g.nextPage());

      break;   
    }

    case 6:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_6();         // Text Graphic Movement using Analog Stick page
      
      } 
      while (u8g.nextPage());

      break;   
    }

  }



}



/////////////////////////////////////////////////////////////////////////



void pressBUT_1 ()      // Using a normal tactile push button
{
    
    
  if (butState_1 != digitalRead(BUT_1))
  {  
    
    if (butState_1 == HIGH)
    {
      butState_1 = digitalRead(BUT_1);
      keyPressed_1 = true;

      previousMillis_1 = currentMillis;
    }
    else if (butState_1 == LOW)
    {
      butState_1 = HIGH;
      keyPressed_1 = false;
    }

  }

  
  if ((butState_1 == LOW) && (keyPressed_1 == true) && ((currentMillis - previousMillis_1) > debounceInterval))
  {

    sendToSlave_1 = 1;
    
    i2c_Tx_Arduinos(9, sendToSlave_1, 1);

    receiveFromSlave_1 =  Wire.read();


    if (receiveFromSlave_1 == sendToSlave_1)
    {
      
      if (stateLED_1 == LOW)
      {
        displayLED_1 = "ON";
        stateLED_1 = HIGH;
      }
      else if (stateLED_1 == HIGH)
      {
        displayLED_1 = "OFF";
        stateLED_1 = LOW;
      }

    }
    else if (receiveFromSlave_1 != sendToSlave_1)
    {
      displayLED_1 = "Com. Error";
    }
    
    
    sendToSlave_1 = 0;
    receiveFromSlave_1 = 0;
    
    
    
    if (digitalRead(BUT_1) == LOW)      // No toggling until the finger has been lifted off
    {
      keyPressed_1 =  false;
      butState_1 = LOW;
    }
    else if (digitalRead(BUT_1) == HIGH)
    {
      keyPressed_1 =  true;
      butState_1 = HIGH;
    }
    

  } 


}



/////////////////////////////////////////////////////////////////////////



void pressBUT_2 ()        // Using a normal tactile push button
{
    
    
  if (butState_2 != digitalRead(BUT_2))
  {  
    
    if (butState_2 == HIGH)
    {
      butState_2 = digitalRead(BUT_2);
      keyPressed_2 = true;

      previousMillis_2 = currentMillis;
    }
    else if (butState_2 == LOW)
    {
      butState_2 = HIGH;
      keyPressed_2 = false;
    }

  }


  if ((butState_2 == LOW) && (keyPressed_2 == true) && ((currentMillis - previousMillis_2) > debounceInterval))
  {

    sendToSlave_2 = 2;

    i2c_Tx_Arduinos(9, sendToSlave_2, 1);

    receiveFromSlave_2 =  Wire.read();

    if (receiveFromSlave_2 == sendToSlave_2)
    {
      
      if (stateLED_2 == LOW)
      {
        displayLED_2 = "ON";
        stateLED_2 = HIGH;
      }
      else if (stateLED_2 == HIGH)
      {
        displayLED_2 = "OFF";
        stateLED_2 = LOW;
      }

    }
    else if (receiveFromSlave_2 != sendToSlave_2)
    {
      displayLED_2 = "Com. Error";
    }


    sendToSlave_2 = 0;
    receiveFromSlave_2 = 0;

    
    if (digitalRead(BUT_2) == LOW)      // No toggling until the finger has been lifted off
    {
      keyPressed_2 =  false;
      butState_2 = LOW;
    }
    else if (digitalRead(BUT_2) == HIGH)
    {
      keyPressed_2 =  true;
      butState_2 = HIGH;
    }
    

  } 
  

}



/////////////////////////////////////////////////////////////////////////



void pressEncoderBUT ()       // Using the button from Rotatory Encoder
{
    
  
  if (butState_3 != digitalRead(encoder_BUT))
  {  
    
    if (butState_3 == HIGH)
    {
      butState_3 = digitalRead(encoder_BUT);
      keyPressed_3 = true;

      previousMillis_3 = currentMillis;
    }
    else if (butState_3 == LOW)
    {
      butState_3 = HIGH;
      keyPressed_3 = false;
    }

  }


  if ((butState_3 == LOW) && (keyPressed_3 == true) && ((currentMillis - previousMillis_3) > debounceInterval))
  {
      
    if (switchDisplay == 1)
    {
      switchDisplay = 2;

      upCounter = 0;     
      downCounter = 100;
    }
    else if (switchDisplay == 2)
    {
      switchDisplay = 3;

      previousMillis_6 = currentMillis;
    }
    else if (switchDisplay == 3)
    {
      switchDisplay = 4;

      previousMillis_6 = currentMillis;
    }
    else if (switchDisplay == 4)
    {
      switchDisplay = 5;

      previousMillis_6 = currentMillis;
    }
    else if (switchDisplay == 5)
    {
      switchDisplay = 6;

      previousMillis_6 = currentMillis;
    }
    else if (switchDisplay == 6)
    {
      switchDisplay = 1;
    }


    
    if (digitalRead(encoder_BUT) == LOW)      // No toggling until the finger has been lifted off
    {
      keyPressed_3 =  false;
      butState_3 = LOW;
    }
    else if (digitalRead(encoder_BUT) == HIGH)
    {
      keyPressed_3 =  true;
      butState_3 = HIGH;
    }
    

  } 


}



/////////////////////////////////////////////////////////////////////////



void encoderISR()     // ISR for taking in the Encoder readings quickly
{
  
  current_Clk_State = digitalRead(encoder_Clk);
  current_Data_State = digitalRead(encoder_Data);


  if (current_Clk_State == LOW)
  {
    
    encoderStatus = current_Data_State;
    previousMillis_4 = currentMillis;

  }

  else if (current_Clk_State == HIGH)
  {

    encoderStatus = !current_Data_State;
    previousMillis_5 = currentMillis;

  }

  encoderBusy = true;
  
}



/////////////////////////////////////////////////////////////////////////



void init_Accel_Gyro ()             // Initializing the MPU 6050's with Custom Settings
{
  
  Wire.beginTransmission(0x68);     // 6050 MPU's i2c Address
  Wire.write(0x6B);                 // Accessing the MPU's power management
  Wire.write(0x0);                  // Activating the 6050 MPU from sleep 
  Wire.endTransmission();


  Wire.beginTransmission(0x68);     // 6050 MPU's i2c Address
  Wire.write(0x1C);                 // Accessing Accelerometer settings
  Wire.write(0x0);                  // Setting sensivity to +- 2 g
  Wire.endTransmission();


  Wire.beginTransmission(0x68);     // 6050 MPU's i2c Address
  Wire.write(0x1B);                 // Accessing Gyroscope settings
  Wire.write(0x0);                  // Setting sensivity to +- 250 degree/sec
  Wire.endTransmission();

}



/////////////////////////////////////////////////////////////////////////



void readingAccel ()
{
  
  Wire.beginTransmission(0x68);     // 6050 MPU's i2c Address
  Wire.write(0x3B);                 // Asking for Accelerometer readings
  Wire.endTransmission();

  Wire.requestFrom(0x68, 6);        // Actually receiving the Accelerometer readings here, total 6-Bytes

  while (Wire.available() < 6)      // Making sure the receiving buffer is filled before reading
  {
    // Do nothing, just make sure the buffer is filled with 6 bytes,
    // 2 for each axis.
  }


  rawAccel.x = Wire.read() << 8;               // Reading 1 byte and making some space
  rawAccel.x = rawAccel.x | Wire.read();       // Reading another byte and taking a union (ORing)

  rawAccel.y = Wire.read() << 8;               // Reading 1 byte and making some space
  rawAccel.y = rawAccel.y | Wire.read();       // Reading another byte and taking a union (ORing)

  rawAccel.z = Wire.read() << 8;               // Reading 1 byte and making some space
  rawAccel.z = rawAccel.z | Wire.read();       // Reading another byte and taking a union (ORing)


  // Note that the 6050 MPU provides a 2-Byte or 16-Bit data for each axis readings, but i2c only
  // sends 1-Byte data from the buffer at a time. So we have to take those two bytes, and "shift-add" them together.

  // Best explained using this example:

  // The 16-Bits or 2-Bytes to be sent using i2c are:
  // 10101010 01010101
  // This is split up in two Bytes for i2c transmission.

  // First Byte that is received: 10101010
  // Shifting the First Byte by 8-Bits to make space for the second one: 10101010 00000000

  // Second Byte is received: 01010101
  // ORing them: 10101010 00000000 + 01010101

  // This results in: 10101010 01010101
  // Which is actually the 16-Bit reading taken by the MPU, but due to i2c limitations, 
  // it is split up in 2-Bytes to be sent seperately.


  // Converting to actual meaningfull values. Dividing by "16384.0" due 
  // to the "+- 2 g" sensitivity settings. Check DataSheet for other sensitivities.

  cookedAccel.x = rawAccel.x / 16384.0;
  cookedAccel.x = cookedAccel.x - 0.06;         // Offset calculated by averaging the readings manually

  cookedAccel.y = rawAccel.y / 16384.0;
  cookedAccel.y = cookedAccel.y - 0.04;         // Offset calculated by averaging the readings manually

  cookedAccel.z = rawAccel.z / 16384.0;         
  cookedAccel.z = cookedAccel.z - 0.09;         // Offset calculated by averaging the readings manually


  if ((cookedAccel.y > 0.02) && (cookedAccel.y <= 0.95))        // This checks the oreintation based
  {                                                             // on Accelorometer data from the MPU 6050
    rightLED_Comm();                                            // blinks the appropriate LED light.

    if (displayLED_1 == "ON")
    {
      leftLED_Comm();       // Turning the other LED OFF
    }
  }

  if ((cookedAccel.y > -1.95) && (cookedAccel.y <= -0.02))
  {
    leftLED_Comm();

    if (displayLED_2 == "ON")
    {
      rightLED_Comm();       // Turning the other LED OFF
    }
  }

  if ((cookedAccel.y > -0.02) && (cookedAccel.y <= 0.02))
  {
    
    if (displayLED_1 == "ON")
    {
      leftLED_Comm();       // Turning the left LED OFF
    }

    if (displayLED_2 == "ON")
    {
      rightLED_Comm();       // Turning the right LED OFF
    }

  }

}



/////////////////////////////////////////////////////////////////////////



void readingGyro ()
{
  
  Wire.beginTransmission(0x68);     // 6050 MPU's i2c Address
  Wire.write(0x43);                 // Asking for Gyroscope readings
  Wire.endTransmission();

  Wire.requestFrom(0x68, 6);        // Actually receiving the Gyroscope readings here, total 6-Bytes

  while (Wire.available() < 6)      // Making sure the receiving buffer is filled before reading
  {
    // Do nothing, just make sure the buffer is filled with 6 bytes,
    // 2 for each axis.
  }
                  

  rawGyro.x = Wire.read() << 8;                // Reading 1 byte and making some space
  rawGyro.x = rawGyro.x | Wire.read();         // Reading another byte and taking a union (ORing)
 

  rawGyro.y = Wire.read() << 8;                // Reading 1 byte and making some space
  rawGyro.y = rawGyro.y | Wire.read();         // Reading another byte and taking a union (ORing)

  rawGyro.z = Wire.read() << 8;                // Reading 1 byte and making some space
  rawGyro.z = rawGyro.z | Wire.read();         // Reading another byte and taking a union (ORing)


  // Note that the 6050 MPU provides a 2-Byte or 16-Bit data for each axis readings, but i2c only
  // sends 1-Byte data from the buffer at a time. So we have to take those two bytes, and "shift-add" them together.

  // Best explained using this example:

  // The 16-Bits or 2-Bytes to be sent using i2c are:
  // 10101010 01010101
  // This is split up in two Bytes for i2c transmission.

  // First Byte that is received: 10101010
  // Shifting the First Byte by 8-Bits to make space for the second one: 10101010 00000000

  // Second Byte is received: 01010101
  // ORing them: 10101010 00000000 + 01010101

  // This results in: 10101010 01010101
  // Which is actually the 16-Bit reading taken by the MPU, but due to i2c limitations, 
  // it is split up in 2-Bytes to be sent seperately.


  // Converting to actual meaningfull values. Dividing by "131.0" due 
  // to the "+- 250 degree/sec" sensitivity settings. Check DataSheet for other sensitivities.

  cookedGyro.x = rawGyro.x / 131.0;
  cookedGyro.x = cookedGyro.x + 4.35;       // Offset calculated by averaging the readings manually

  cookedGyro.y = rawGyro.y / 131.0;
  cookedGyro.y = cookedGyro.y - 2.05;       // Offset calculated by averaging the readings manually

  cookedGyro.z = rawGyro.z / 131.0;
  cookedGyro.z = cookedGyro.z + 0.50;       // Offset calculated by averaging the readings manually

}



/////////////////////////////////////////////////////////////////////////



void leftLED_Comm ()        // For blinking the left LED in reaction to
{                           // spacial position data from Accelorometer
  
  sendToSlave_1 = 1;
  
  i2c_Tx_Arduinos(9, sendToSlave_1, 1);

  receiveFromSlave_1 =  Wire.read();


  if (receiveFromSlave_1 == sendToSlave_1)
  {
    
    if (stateLED_1 == LOW)
    {
      displayLED_1 = "ON";
      stateLED_1 = HIGH;
    }
    else if (stateLED_1 == HIGH)
    {
      displayLED_1 = "OFF";
      stateLED_1 = LOW;
    }

  }
  else if (receiveFromSlave_1 != sendToSlave_1)
  {
    displayLED_1 = "Com. Error";
  }
  
  
  sendToSlave_1 = 0;
  receiveFromSlave_1 = 0;

}


/////////////////////////////////////////////////////////////////////////



void rightLED_Comm ()       // For blinking the left LED in reaction to
{                           // spacial position data from Accelorometer

  sendToSlave_2 = 2;

  i2c_Tx_Arduinos(9, sendToSlave_2, 1);

  receiveFromSlave_2 = Wire.read();


  if (receiveFromSlave_2 == sendToSlave_2)
  {
    
    if (stateLED_2 == LOW)
    {
      displayLED_2 = "ON";
      stateLED_2 = HIGH;
    }
    else if (stateLED_2 == HIGH)
    {
      displayLED_2 = "OFF";
      stateLED_2 = LOW;
    }

  }
  else if (receiveFromSlave_2 != sendToSlave_2)
  {
    displayLED_2 = "Com. Error";
  }


  sendToSlave_2 = 0;
  receiveFromSlave_2 = 0;

}



/////////////////////////////////////////////////////////////////////////



void drawPage_1 ()       // For displaying LED Status page
{
  
  u8g.setFont(u8g_font_helvB08r); 

  u8g.drawStr(45, 11, "i2c Coms");
  
  u8g.drawStr(0, 35, "Yeeloow LED:");
  u8g.setPrintPos(85, 35); 
  u8g.print(displayLED_1);
    
  u8g.drawStr(0, 52, "Gereene LED:");
  u8g.setPrintPos(85, 52); 
  u8g.print(displayLED_2);
  
}



/////////////////////////////////////////////////////////////////////////



void drawPage_2 ()       // For displaying Counter page
{
  
  u8g.setFont(u8g_font_6x13); 

  u8g.drawStr(20, 11, "Encoder Counter");
  
  u8g.drawStr(0, 35, "Up Counter:");
  u8g.setPrintPos(102, 35); 
  u8g.print(upCounter);
    
  u8g.drawStr(0, 52, "Down Counter:");
  u8g.setPrintPos(102, 52); 
  u8g.print(downCounter);
  
}



/////////////////////////////////////////////////////////////////////////



void drawPage_3 ()       // For displaying 6050 MPU Accel page
{
  
  u8g.setFont(u8g_font_6x13); 

  u8g.drawStr(15, 10, "Accelerometer");
  
  u8g.drawStr(0, 28, "X-Axis:");
  u8g.setPrintPos(60, 28); 
  u8g.print(cookedAccel.x);
    
  u8g.drawStr(0, 46, "Y-Axis:");
  u8g.setPrintPos(60, 46); 
  u8g.print(cookedAccel.y);

  u8g.drawStr(0, 64, "Z-Axis:");
  u8g.setPrintPos(60, 64); 
  u8g.print(cookedAccel.z);
  
}



/////////////////////////////////////////////////////////////////////////



void drawPage_4 ()       // For displaying 6050 MPU Gyro page
{
  
  u8g.setFont(u8g_font_6x13); 

  u8g.drawStr(25, 10, "Gyroscope");
  
  u8g.drawStr(0, 28, "X-Axis:");
  u8g.setPrintPos(60, 28); 
  u8g.print(cookedGyro.x);
    
  u8g.drawStr(0, 46, "Y-Axis:");
  u8g.setPrintPos(60, 46); 
  u8g.print(cookedGyro.y);

  u8g.drawStr(0, 64, "Z-Axis:");
  u8g.setPrintPos(60, 64); 
  u8g.print(cookedGyro.z);
  
}



/////////////////////////////////////////////////////////////////////////



void drawPage_5 ()       // For displaying Analog Stick's Axis Data
{
  
  u8g.setFont(u8g_font_6x13); 

  u8g.drawStr(10, 10, "Analog Stick Reader");
  
  /*u8g.drawStr(0, 35, "X-Axis:");
  u8g.setPrintPos(85, 35); 
  u8g.print(analogStick.x);
    
  u8g.drawStr(0, 60, "Y-Axis:");
  u8g.setPrintPos(85, 60); 
  u8g.print(analogStick.y);*/

  u8g.drawStr(0, 28, "X-Axis:");
  u8g.setPrintPos(60, 28); 
  u8g.print(analogStick.x);
    
  u8g.drawStr(0, 46, "Y-Axis:");
  u8g.setPrintPos(60, 46); 
  u8g.print(analogStick.y);

  u8g.drawStr(0, 64, "Button:");
  u8g.setPrintPos(60, 64); 
  u8g.print(analogStick.receiveButState);
  
}



/////////////////////////////////////////////////////////////////////////



void drawPage_6 ()       // For displaying Text Graphic's movement using Analog Stick
{
  
  u8g.setFont(u8g_font_6x13); 
  
  u8g.drawStr(analogStick.x, analogStick.y, "|Y|");   
}



/////////////////////////////////////////////////////////////////////////



long rangeMapper (double value, double fromMin, double fromMax, double toMin, double toMax)     // Just a mapping functon
{                                                                                               // that can be modified  
  double fromRange = fromMax - fromMin;                                                         // to accept more data types
  double toRange = toMax - toMin;

  long converted = 0;

  converted = (value - fromMin) * (toRange / fromRange) + toMin;

  return (converted);
}



/////////////////////////////////////////////////////////////////////////



void analogStickReception ()        // Function for receiving RAW analog data for
{                                   // Analog Stick using i2c from the Slave Arduino

  sendToSlave_3 = 3;
  
  i2c_Tx_Arduinos(9, sendToSlave_3, 5);
  
  analogStick.receiveButState = Wire.read();
  
  analogStick.x = Wire.read() << 8;
  analogStick.x = analogStick.x | Wire.read();

  analogStick.y = Wire.read() << 8;
  analogStick.y = analogStick.y | Wire.read();


  analogStick.x = rangeMapper(analogStick.x, -9, 1014, 110, 0);     // Range values determined experimently

  analogStick.y = rangeMapper(analogStick.y, 12, 1034, 10, 64);     // Range values determined experimently

}


/////////////////////////////////////////////////////////////////////////



void analogStickSwipe ()        // Funtion that allows to change display pages by swiping the Analog Stick
{

  if (analogStick.receiveButState == 1)
  {

    if ((analogStick.x > 67) && (analogStick.x <= 110))     // Swiping Right
    {
      switchDisplay = switchDisplay + 1;

      previousMillis_6 = currentMillis;       // Starting the common timer
      
      if (switchDisplay > 6)
      {
        switchDisplay = 1;
      }

    }


    if ((analogStick.x < 45) && (analogStick.x >= 0))       // Swiping Left
    {
      switchDisplay = switchDisplay - 1;

      previousMillis_6 = currentMillis;       // Starting the common timer
      
      if (switchDisplay < 1)
      {
        switchDisplay = 6;
      }

    }

  }


}



/////////////////////////////////////////////////////////////////////////



void i2c_Tx_Arduinos (byte Address, byte DataToBeSent, byte Rx_DataSizeinBytes)      // Simple i2c Transmission function that replaces
{                                                                                    // repetative lines of code with this function call

  Wire.beginTransmission(Address); 
  Wire.write(DataToBeSent);  
  Wire.endTransmission();

  Wire.requestFrom(Address, Rx_DataSizeinBytes);

}



/////////////////////////////////////////////////////////////////////////