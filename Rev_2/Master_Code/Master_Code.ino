

///// Rev_2, Master Code /////
///// Added a Rotatory Encoder /////
///// Rotatory Encoder is connected to the Master Arduino /////
///// Master Arduino sends the Rotatory Emcoder input to the Slave Arduino /////
///// Slave Arduino reacts to the Encoder input by turing the appropriate LED "ON" according to the Encoder's direction /////
///// The OLED Display connected to the Master display also shows the Encoder's directional input in the form of Up-Counter and Down-Counter /////
///// The two Push Buttons now only give input once when pressed and then only after the finger is lifted and the Button is pressed again /////

///// GitHub: @Hizbi-K /////



#include <Arduino.h>
#include <Wire.h>
#include <U8glib.h>


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 



byte sendToSlave_1 = 0;
byte receiveFromSlave_1 = 0;

byte sendToSlave_2 = 0;
byte receiveFromSlave_2 = 0;


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

#define INTERRUPT 0

volatile bool encoderStatus = HIGH;
volatile bool encoderBusy = false;

int encoder_BUT = 4;
bool butState_3 = LOW;
bool keyPressed_3 = false;


volatile int upCounter = 0;
volatile int downCounter = 100;


int switchDisplay = 0;


String displayLED_1 = "OFF";
String displayLED_2 = "OFF";


unsigned long currentMillis = 0;
unsigned long previousMillis_1 = 0;
unsigned long previousMillis_2 = 0;
unsigned long previousMillis_3 = 0;
unsigned long previousMillis_4 = 0;
unsigned long previousMillis_5 = 0;

unsigned long debounceInterval = 50;
unsigned long encoderDebounce = 50;


void pressBUT_1 ();
void pressBUT_2 ();
void pressEncoderBUT ();

void encoderISR ();

void leftLED_Comm ();
void rightLED_Comm ();

void drawPage_1 ();
void drawPage_2 ();



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


    Wire.begin(); 
  
}



void loop() 
{

  currentMillis = millis();


  pressBUT_1();

  pressBUT_2();

  pressEncoderBUT();


  if (switchDisplay == 1)                               // Only using the encoder if the display
  {                                                     // has been switched to the Counter page
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

      encoderBusy = false;             // Allowing Arduino to read Encoder again

    }

  }



  switch(switchDisplay)       // Switching between the LED Status and Counter pages
  {

    case 0:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_1();         // LED Status page
      
      } 
      while (u8g.nextPage());

      break;
    }

    case 1:
    {
      u8g.firstPage();  

      do 
      {  
        drawPage_2();         // Counter page
      
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
      sendToSlave_1 = 0;
      
      Wire.beginTransmission(9); 
      Wire.write(sendToSlave_1);  
      Wire.endTransmission();

      Wire.requestFrom(9,1);
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
      sendToSlave_2 = 1;
      
      Wire.beginTransmission(9); 
      Wire.write(sendToSlave_2);  
      Wire.endTransmission();
      

      Wire.requestFrom(9,1);
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
        
      if (switchDisplay == 0)
      {
        switchDisplay = 1;
      }
      else if (switchDisplay == 1)
      {
        switchDisplay = 0;
      }


      upCounter = 0;     
      downCounter = 100;
            
      
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



void leftLED_Comm ()
{
  
  sendToSlave_1 = 0;
      
  Wire.beginTransmission(9); 
  Wire.write(sendToSlave_1);  
  Wire.endTransmission();

  Wire.requestFrom(9,1);
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



void rightLED_Comm ()
{

  sendToSlave_2 = 1;
      
  Wire.beginTransmission(9); 
  Wire.write(sendToSlave_2);  
  Wire.endTransmission();
  

  Wire.requestFrom(9,1);
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

}



/////////////////////////////////////////////////////////////////////////



void drawPage_1()       // For displaying LED Status page
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



void drawPage_2()       // For displaying Counter page
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







/////////////////////////////////////////////////////////////////////////
















