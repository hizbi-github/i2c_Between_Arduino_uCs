
# i2c_Between_Arduinos

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Front_Page.jpg" />
</p>


# Introduction

This is a beginner friendly project for those interested in using i2c or IIC protocol with Arduino Master and Arduino Slave.

The use of other i2c peripherals such as 0.96" OLED display and MPU 6050 sensor has also been demonstrated in this code.

Furthermore, this project also includes code for using a Rotary Encoder, an Analog Stick, push-buttons and LED lights with Arduino.

All these major components types (10 in total) include:

1. One Arduino Nano as i2c Master
2. One Arduino Pro Micro (5V) as i2c Slave
3. One 0.96" i2c OLED Display
4. One MPU 6050 Gyroscope and Accelerometer
5. One Rotary Encoder
6. One Analog Stick (like those found in old PS2 controllers)
7. Three Generic Push Buttons
8. Two 5mm LED Lights
9. Lots of Jumper Cables (both M-M and M-F)
10. 5V Power Supply

Note: I used two 18650 Li-Ion Cells connected in Series to give 7.4V in total. The power supply was then connected to RAW Voltage 
pins for both the Arduinos. The rest of the components were directly powered from the regulated 5V pin of the Arduinos, since they dont
draw too much power anyways. Remember to always use the seperate power supply for power hungry components such as motors, etc.


# Images

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Zoomed_In_Complete_Image.jpg" />
</p>

<p align="center">
  All Components fixed on a Breadboard
</p>


<p align="center">
  <img src="Demonstration_Images_and_Gifs\Two_Arduinos.jpg" />
</p>

<p align="center">
  Arduino Nano as i2c Master (Left) and Arduino Pro Micro as i2c Slave (Left)
</p>


# Code Explained

The code is divided into two parts: "Master_Code" and "Slave_Code" which has to be uploaded to the master and slave Arduino seperatly.

I used the Arduino Nano as the i2c Master, while the Arduino Pro Micro became the i2c Slave.

As seen in the block diagramm, the master interacts with 3 individual components directly:

1. the Rotary Encoder
2. and two Push Buttons.

The other devices are connected to the master using i2c Bus:

1. Arduino Pro Micro as i2c Slave,
2. OLED Display,
3. and MPU 6050 Sensor.

On the other hand, the components connected directly to the slave Arduino are:

1. Analog Stick,
2. two LED lights
3. and one Push Button (for resetting the Arduino Pro Micro when uploading the code, if using Arduino Nano as an i2c Slave, this button is not required).

## Master_Code

The functions implemented in the Master Arduino can summarized as:

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Switching_LED_Lights_using_Buttons.gif" />
</p>

<p align="center">
  Switching LED Lights using Buttons
</p>

1.      void pressBUT_1 ();

        void pressBUT_2 ();

    These two fucntions are same. First one is used to detect whether Button 1 is pressed or not and the second fucntion detects the Button 2. Both of these buttons include Software Debounce for the push buttons, which prevents extra unwanted button presses whenever a button is pressed or released. 

    These fucntions also use the i2c Bus to signal the Slave Arduino to turn ON or OFF the respective LED lights connected to it.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Different_Display_Pages.gif" />
</p>

<p align="center">
  Different Display Pages
</p>

2.      void pressEncoderBUT ();

    Pretty much the same as the above two fucntions. This fucntions reads the Rotary Encoder's Button and is used to switch between the different display pages on the OLED screen. 

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Rotary_Encoder_Reaction.gif" />
</p>

<p align="center">
  LED Lights React to the Rotary Encoder's Direction
</p>

3.      void encoderISR ();

    This is an Interrup Serivce Routine which reads the interrupt from the Rotary Encoder (only one pin) to detect its direction. The UP-Down Counter shown on the display reacts to the Rotary Encoder's direction, while the LED lights also blink in the appropriate direction.

4.      void init_Accel_Gyro ();

    This function is used to initialize the MPU 6050 with custom configuration settings. This is done by setting the values of the specific registors on this sensor.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\LED_Reaction_due_to_Orientation_using_Accel.gif" />
</p>

<p align="center">
  LEDs' Reaction due to Orientation using Accelerometer
</p>

5.      void readingAccel();
        void readingGyro();

    The Accelerometer and Gyroscope data from the MPU 6050 on the i2c Bus is accessed using these two fucntions. This is done by first requesting the appropriate registor to read from, and then reading the data sent over by the MPU. The data is sent Bit-by-Bit in i2c transmissions. Therefore, on reception the data is combined back together. Offsets are also applied. The LEDs reaction process is also performed on the Accelerometer data to determine the current orientation, and the result is shown by blinking the two LEDs.

6.      void leftLED_Comm ();
        void rightLED_Comm ();     

    These two functions perform the task of sending a signal to the slave Arduino to blink the appropriate LED light when the function is called.

7.      void drawPage_1 ();       Status of the two LEDs
        void drawPage_2 ();       Up-Down Counter for Rotary Encoder
        void drawPage_3 ();       Accelerometer Axis Data
        void drawPage_4 ();       Gyroscope Axis Data
        void drawPage_5 ();       Analog Stick Axis Data
        void drawPage_6 ();       Moving the Text Graphic using the Analog Stick

    The OLED display is used with "U8glib" library. To render something on the display, a page function has to be created. Only content of a single page function is displayed at once. Each of these six fucntions represent a different page as described next to the each function.

8.      void i2c_Tx_Arduinos (int Address, int DataToBeSent, int DataSizeinBytes);

    Yet to be implemented. This function will simplify the i2c Transmission process and the overall code length. An i2c Transmission is done by first selecting the slave address, then sending it the registor Byte to tell the Slave what to send. This is done because the slave is unable to send anything by itself, only when the master request some data, the slave can then send it. Next, the data is actually requested from the slave using its i2c Address and the number of Bytes the master expects to receive. Instead of doing this everytime, this function will be called which will perform the i2c Transmission

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Moving_Text_Graphic_Using_Analog_Stick.gif" />
</p>

<p align="center">
  Moving Text Graphic Using Analog Stick
</p>

9.      void analogStickReception ();

    This fucntion requests the RAW Analog Stick readings from the slave Arduino. The data is then received in Bytes and combined together. After that, it is mapped to the display resolution of the OLED Screen. This mapped data is shown on the page # 5 of the OLED display. This also allows the Text Graphic to move across the screen without any clipping using the Analog Stick on the slave Arduino.

10.     long rangeMapper (double value, double fromMin, double fromMax, double toMin, double toMax);

    A function for mapping two different ranges of data together based on the minimum and maximum values of both ranges. The current value in the range from which mapping is performed is also required. Here this function is used for mapping the RAW Analog Stick data from the slave Arduino to the display resolution of OLED display. 

## Slave_Code

The slave code is far too simple, considering the master Arduino is doing all the heavy-lifting. There are only three functions here:

1.      void analogStickReader ();

    This function simply simply reads the Analog Stick data for both X and Y-Axis and stores them in a struct. The offsets, which were calculated manually, are also applied to get accurate data.

2.      void slaveRead(int);

    This function is used when the master Arduino sends data on the i2c Bus. This triggers the ISR for the slave Arduino, where this function is called. Since this happens inside the ISR, this function must be short so that the duration of the ISR is short as possible. The data is sent in Bytes from the master. Therefore, if there are more than one Bytes of data, it has to be stored inside a buffer and then later combined if required. The single "int" parameter is passed from the i2c ISR and it tells the slave how many Bytes are expected to be received from the master Arduino.

3.      void slaveWrite();

    This function is used to send the data to the master Arduino when requested. For a single Byte, the "Wire.write()" is called once and the data is passed to it. For sending multiple Bytes, a buffer is required. The buffer is filled Byte-by-Byte and then the buffer and its size in Bytes is passed to the "Wire.write()" function. The master Arduino specifies the number of Bytes the master will receive. Even if the slave tries to send more, the master Arduino will ignore them.


# Regarding i2c Communication and Bit-Shifting (For i2c Sensors and Devices e.g MPU 6050)

Note that the 6050 MPU provides a 2-Byte or 16-Bit data for each axis readings, but i2c only sends 1-Byte data from the buffer at a time. So we have to take those two bytes, and "shift-add" them together.

Best explained using this example:

The 16-Bits or 2-Bytes to be sent using i2c are:

10101010 01010101

This is split up in two Bytes for i2c transmission.

1. First Byte that is received: 10101010

    Shifting the First Byte by 8-Bits to make space for the second one: 10101010 00000000

2. Second Byte is received: 01010101

    ORing them: 10101010 00000000 + 01010101

This results in: 10101010 01010101

Which is actually the 16-Bit reading taken by the MPU, but due to i2c limitations, it is split up in 2-Bytes to be sent seperately.


# Next Version

The code has been commented properly to expain each function and block of code, but the code can still be simplified further by turning the repetative lines of code into seperate functions. So far, there are no bugs in the code and the hardware responds within the expectations. The next version will introduce more functions to replace some repetative code and make use of the Button in Analog Stick. A new display-page selection menu will also be implemented.



<p align="center">
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
</p>