#include <GPS_lib.h>
#include <SPI.h>
#include <SD.h>

File myFile;

#define trigPin1 13
#define echoPin1 12
#define ledPin 4 
#define controlledPin 5 
#define GPScontrolledPin 6
#define startbuttonPin 2
#define endbuttonPin 3

unsigned long czas;
int temp_time = 5000;

int buttonState = LOW; 
String IDcodeString = "";
String IDcodeprev = "";
int counter = 0;
long lastDebounceTime = 0;
long lastDebounceTimeStop = 0;
long debounceDelay = 50;

boolean isWorking = false;

char buff[82];

GPS my_gps;
char ch;

String dataString = "";

void setup()
{
  pinMode(trigPin1, OUTPUT); 
  pinMode(echoPin1, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(controlledPin, OUTPUT);
  pinMode(GPScontrolledPin, OUTPUT);
  pinMode(startbuttonPin, INPUT);
  pinMode(endbuttonPin, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(endbuttonPin), endWorking, RISING);
  
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }
  if (!SD.begin(53)) {
    Serial.println("initialization of SD card failed!");
    return;
  }
   Serial1.begin(9600);
  if (!Serial1) {
    Serial.print("Initialization of RFID failed");
    return;
  }
  Serial2.begin(9600);
  if (!Serial2) {
    Serial.print("Initialization of GPS failed");
    return;
  }
  delay(1000);
/*
while(1)
{
if (SD.exists("testgpss.txt")) {
    Serial.println("testgpss.txt exists.");
    break;
  } 
  else {
    Serial.println("testgpss.txt doesn't exist.");
    myFile = SD.open("testgpss.txt", FILE_WRITE);
  delay(500);
  if (myFile) {
    myFile.println("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
    myFile.println("$GPGSA,A,3,3,6,27,19,9,14,21,22,18,15,,,2.1,1.0,1.8*03");
    myFile.println("$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76");
    myFile.println("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A");   
  myFile.close();
delay(500);
}  
  }
}
*/
while(1)
{
if (SD.exists("data.txt")) {
    Serial.println("data.txt exists.");
    break;
  } 
  else {
     delay(500);
    Serial.println("data.txt doesn't exist.");
    myFile = SD.open("data.txt", FILE_WRITE);
    delay(500);
    if (myFile)
    {
      myFile.println("beginning");  
      myFile.close();
    }

}  
}

delay(500);
 while(1)
  {
    if (startWorking())
    {
      buttonState = LOW;
      isWorking = true;
      break;
    }  
  } 
  delay(500);
  Serial.println("while finished !!!");
 
}
void loop()
{
  if (isWorking == true)
  { 
          if (Serial1.available() > 0) 
      { 
         delay (500); 
         IDcodeprev = IDcodeString;
         IDcodeString = "";
        for (int z = 0 ; z < 14 ; z++)
        {   
         // IDcode = IDcode+ Serial1.read();
         IDcodeString+= Serial1.read();
         //Serial.println((String)Serial1.read());
        }
        Serial.println("ID code:"); 
        Serial.println(IDcodeString); 
        if (IDcodeString != IDcodeprev )
        {
           digitalWrite(controlledPin, HIGH);
           delay(700);
           digitalWrite(controlledPin, LOW);
           delay(700);
           digitalWrite(controlledPin, HIGH);
           delay(700);
           digitalWrite(controlledPin, LOW);
           temp_time = 10000;
           czas = millis(); 
           saveNewInfo(0);
        }
      }
      else 
      {
     while(Serial2.available())
    {
        if (my_gps.encoded(Serial2.read()))
        {
          Serial.println(my_gps.new_sentence); 
          my_gps.encode();
          //gps_debug();
        }
         
    }
      }
     if (checkDistanceSensor())
     {
       saveNewInfo(1);
       isWorking = false;
     }
     if (millis() - czas > temp_time) 
     {
        saveNewInfo(2);
        temp_time = 5000;
        czas = millis();
     }
     if (my_gps.is_working  == true)
        digitalWrite(GPScontrolledPin, LOW);
     else
        digitalWrite(GPScontrolledPin, HIGH);
  }
}
int zmierzOdleglosc() {
  long czas, dystans;
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
 
  czas = pulseIn(echoPin1, HIGH);
  dystans = czas / 58;
  return dystans;
}
  boolean checkDistanceSensor()
  {
   if (zmierzOdleglosc()>10)
      {
        for (int x = 0 ; x < 10 ; x++)
        {
          Serial.println(x);
          delay(500);
          if (zmierzOdleglosc()<10) 
            return false;
        }
         digitalWrite(controlledPin, HIGH);
         delay(700);
         digitalWrite(controlledPin, LOW);
         delay(700);
         digitalWrite(controlledPin, HIGH);
         delay(700);
         digitalWrite(controlledPin, LOW); 
         digitalWrite(ledPin, LOW);
         return true;
      }
      else return false; 
   
  }
void saveNewInfo( int i)
{
  myFile = SD.open("data.txt", FILE_WRITE);
  delay(100);
  if (myFile) 
  {
      dataString = "";
      dataString += String(my_gps.time_fix);
      dataString += "#";
      dataString += String(my_gps.locNS);
      dataString +="#";
      dataString += String(my_gps.locWE);
      dataString +="#";
    if (i == 0) //pełne info razem z IDcodem - załadowanie śmieci
    {
      dataString += String(IDcodeString);
      dataString +="#";
    }
    if (i == 1) // info z rozładowania śmieci
    {
      dataString +="!!!#";
    }
     dataString += String(my_gps.is_working);
     dataString +="#";
    Serial.println(dataString);
    myFile.println(dataString);
    myFile.close();
    delay(100);
  }  
}

void endWorking() 
{
  buttonState = digitalRead(endbuttonPin);
  if ( (millis() - lastDebounceTimeStop) > debounceDelay) 
  {
    if ( (buttonState == HIGH) ) 
    {
      digitalWrite(ledPin, LOW);
      isWorking = false;
      lastDebounceTime = millis(); 
    }
  }
 Serial.println("przetwalem");
}  
void gps_debug()
{
 //myFile = SD.open("testgpss.txt");
  //if (myFile) {
  /*
    while (myFile.available()) {
     ch = myFile.read();
      if (ch == '$')
        {
            my_gps.char_counter = 0;
            if (my_gps.sentence_updated)
            {
              Serial.println(my_gps.new_sentence);
                my_gps.encode();
                if (my_gps.loc_updated)
                {
                  dataString = "";
                  dataString += String(my_gps.locNS);
                  dataString +=" ";
                  dataString += String(my_gps.locWE);
                  Serial.println(dataString);
                }
 
                if (my_gps.time_updated)
                {
                  Serial.println(my_gps.time_fix);
                }
            }  
        }
        else
        {
           my_gps.fillNewSentence(ch);
        }  
    }
    my_gps.encode();
    */
    if (my_gps.loc_updated)
                {
                  dataString = "";
                  dataString += String(my_gps.locNS);
                  dataString +=" ";
                  dataString += String(my_gps.locWE);
                  //Serial.println(dataString);
                   //cout << my_gps.locNS_deg_int << " deg " << my_gps.locNS_min_float << " ' " << my_gps.NS<< ",  " << my_gps.locWE_deg_int << " deg " << my_gps.locWE_min_float << " ' " << my_gps.WE << "  " <<endl;
                }
 
                if (my_gps.time_updated)
                {
                  //Serial.println(my_gps.time_fix);
                    //cout << my_gps.time_hour_int << " h " << my_gps.time_minute_int << " min " << my_gps.time_sec_int << " sec " << endl << endl;
                }
     
    //}
   //myFile.close(); 
}
boolean startWorking()
{
  buttonState = digitalRead(startbuttonPin);
  if ( (millis() - lastDebounceTime) > debounceDelay) 
  {
    if ( (buttonState == HIGH) ) 
    {
      digitalWrite(ledPin, HIGH);
      lastDebounceTime = millis(); 
      Serial.println("aaa");
      return true;
    }
    else return false;
  }
  else return false;
}

