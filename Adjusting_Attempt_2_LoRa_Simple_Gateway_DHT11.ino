/*
  LoRa Simple Arduino Server :
  Support Devices: 
  * LoRa Mini Dev
  * LoRa Shield + Arduino;
  * LG01

  Software Requirement:
  1/ Install the Radiohead Library(http://www.airspayce.com/mikem/arduino/RadioHead/) to Arduino. 
  
  Example sketch showing how to create a simple messageing gateway,
  This sketch will get the data from LoRa Node and print out the data
  Connection and result please refer: http://wiki.dragino.com/index.php?title=LoRa_Mini#Example_2:_Multi_LoRa_nodes_simple_connection_--_RadioHead_Lib 

  It is designed to work with the other example LoRa_Simple_Client_DHT11

  modified 25 Mar 2017
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited

modified 3 Nov 2019
  by Zachary DeMeglio 
  Pace University Blue CoLab
  
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <FileIO.h>

//When use LG01, uncomment this line, so print the result in Console. 
//When use LoRa Mini Dev, Comment this link
#define  LG01_GATEWAY  

#ifdef LG01_GATEWAY
#include <Console.h>
#include <Process.h>
#define BAUDRATE 115200 
#define SerialPrint Console
#else
#define SerialPrint Serial
#endif

String humidityq = "";
String temperatureq = "";

int led = A2;
float frequency = 915.1; // Change the frequency here.

RH_RF95 rf95;
void setup() 
{
  FileSystem.begin();
  #ifdef LG01_GATEWAY
    Bridge.begin(BAUDRATE);
    SerialPrint.begin();
  #else
    SerialPrint.begin(9600);
  #endif 
  
  if (!rf95.init())
    {
      SerialPrint.println("LoRa module init failed, Please cehck hardware connection");
      while(1) ;
    }
    
  // Setup ISM frequency
  rf95.setFrequency(frequency);
 
  // Setup Power,dBm
  rf95.setTxPower(13);
  
  // Defaults BW Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  SerialPrint.print("Listening on frequency: ");
  SerialPrint.println(frequency);
  
  SerialPrint.println("Filesystem datalogger\n");
}

  // This function return a string with the time stamp
// LG01 will call the Linux "date" command and get the time stamp
String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command

  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }
  
  return result;
}
void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
     if(buf[0] == 1 & buf[1] == 1 & buf[2] ==1) //Get sensor data from node id 111
      {
        int newData[4] = {0, 0, 0, 0};
        for (int i = 0; i < 2; i++)
        {
          newData[i] = buf[i + 3];
        }
        float a = newData[0];
        float b = newData[1];
        humidityq = String(a);
        temperatureq = String(b);
        SerialPrint.print("ID = 111 :");
        SerialPrint.print("Current humdity = ");
        SerialPrint.print(a);
        
        SerialPrint.print("%  ");
        SerialPrint.print("temperature = ");
        SerialPrint.print(b);
        SerialPrint.println("C  ");
      }
     if(buf[0] == 1 & buf[1] == 1 & buf[2] == 2) //Get sensor data from node id 112
      {
        int newData[4] = {0, 0, 0, 0};
        for (int i = 0; i < 2; i++)
        {
          newData[i] = buf[i + 3];
        }
        int a = newData[0];
        int b = newData[1];
        SerialPrint.print("ID = 112 :");
        SerialPrint.print("Current humdity = ");
        SerialPrint.print(a);
        SerialPrint.print("%  ");
        SerialPrint.print("temperature = ");
        SerialPrint.print(b);
        SerialPrint.println("C  ");
      }
    }
    else
    {
        SerialPrint.println("recv failed");
    }
  }



  // make a string that start with a timestamp for assembling the data to log:
  String dataString = "";
  dataString = getTimeStamp();
  dataString += " , ";

  // read three sensors and append to the string:

  dataString += humidityq;
  dataString += String(","+ temperatureq);
  
//  for (int analogPin = 0; analogPin < 3; analogPin++) {
//    int sensor = analogRead(analogPin);
//    dataString += String(sensor);
//    if (analogPin < 2) {
//      dataString += ",";  // separate the values with a comma
//    }
//  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // The FileSystem card is mounted at the following "/mnt/FileSystema1"
  
  File dataFile = FileSystem.open("/mnt/FileSystema1/data/datalog.csv", FILE_APPEND);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    SerialPrint.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    SerialPrint.println("error opening datalog.csv");
  } 
  
  delay(15000);

  
}
