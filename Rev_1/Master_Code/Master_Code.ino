
#include <Arduino.h>
#include <Wire.h>
#include <U8glib.h>


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 



int sendToSlave = 0;


int BUT_1 = A0;
bool butState_1 = LOW;
bool keyPressed_1 = false;
bool stateLED_1 = LOW;

int BUT_2 = A1;
bool butState_2 = HIGH;
bool keyPressed_2 = false;
bool stateLED_2 = LOW;


String displayLED_1 = "OFF";
String displayLED_2 = "OFF";
//String funny;


unsigned long currentMillis = 0;
unsigned long previousMillis_1 = 0;
unsigned long previousMillis_2 = 0;

unsigned long debounceInterval = 50;


void pressBUT_1 ();
void pressBUT_2 ();
void draw ();



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
    
    Wire.begin(); 
  
}



void loop() 
{

  currentMillis = millis();


  pressBUT_1();

  pressBUT_2();


  u8g.firstPage();  

  do 
  {  
    draw();
  
  } while( u8g.nextPage() );
      
  //delay(50);
  
}



/////////////////////////////////////////////////////////////////////////



void pressBUT_1 ()
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
      sendToSlave = 1;
      
      Wire.beginTransmission(9); 
      Wire.write(sendToSlave);  
      Wire.endTransmission();
      
      sendToSlave = 0;

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

      
      if (digitalRead(BUT_1) == LOW)
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



void pressBUT_2 ()
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
      sendToSlave = 2;
      
      Wire.beginTransmission(9); 
      Wire.write(sendToSlave);  
      Wire.endTransmission();
      
      sendToSlave = 0;

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

      
      if (digitalRead(BUT_2) == LOW)
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



void draw() 
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


























