#include <GPS_lib.h>
#include <SD.h>

File myFile;

#define trigPin1 13                 // pin "ECHO" czujnika odległości
#define echoPin1 12                 // pin "TRIG" czujnika odległości
#define ledPin 4                    // pin diody zaczynającej świecić przy uruchomieniu systemu
#define controlledPin 5             // pin diody migającej w przypadku wystąpienia wyładowania odpadów
#define GPScontrolledPin 6          // pin diody świecącej w przypadku niedziałania modułu GPS
#define startbuttonPin 2            // pin pin przycisku zaczynającego pracę (START)
#define endbuttonPin 3              // pin pin przycisku kończącego pracę (STOP)

unsigned long RFID_time;            // czas pomiędzy poszczególnymi odczytami 'breloczków' RFID
unsigned long counter_error;        // czas od ostatniego poprawnego odczytu z modułu GPS
int temp_time = 5000;               // maksymalny czas pomiędzy odczytami 'breloczków' RFID
int temp_time_error = 20000;        // maksymalny czas pomiędzy odczytami z modułu GPS

int buttonState = LOW;              // zmienna zapisująca poczatkowy stan przycisku (do następującego po tej czynności sprawdzania drgań) 
String IDcodeString = "";           // string do zapisu kodu z czytnika RFID
String IDcodeprev = "";             // string do przechowywania poprzedniego kodu z czytnika RFID
long lastDebounceTime = 0;          // zmienna wykorzystwana do eliminowania drgań przycisku START
long lastDebounceTimeStop = 0;      // zmienna wykorzystwana do eliminowania drgań przycisku STOP
long debounceDelay = 50;            // czas wykorzystywany w eliminowaniu drgań przycisków

boolean isWorking = false;          // zmienna określająca czy system pracuje czy jest wyłączony

GPS my_gps;                         // obiekt klasy GPS

String dataString = "";             // string służący do łączenia równych informacji i zapisywania ich później do pliku


/* 
 * funkcja SETUP - wykonuje się tylko raz, ustawiono tam piny i zainicjowano porty stworzono plik tekstowy na karcie SD i oczekiwano wciśnięcia przycisku START 
 */
void setup()
{
  /* 
   * pinMode(NR_PINU, OUTPUT | INPUT) - ustawianie poszczególnych pinów jako wyjścia (OUTPUT) lub wejścia (INPUT) 
   */
  pinMode(trigPin1, OUTPUT); 
  pinMode(echoPin1, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(controlledPin, OUTPUT);
  pinMode(GPScontrolledPin, OUTPUT);
  pinMode(startbuttonPin, INPUT);
  pinMode(endbuttonPin, INPUT);
  
  /*
   * włączenie przerwania - dla pinu podpiętego pod przycisk kończacy pracę systemu, wywołuje funkcję 'endWorking' 
   */
  attachInterrupt(digitalPinToInterrupt(endbuttonPin), endWorking, RISING);

  /* 
   * inicjalizowanie portu szeregowego podłączonego do komputera - w celu debugowania, w finalnej wersji port ten nie będzie inicjalizowany 
   */
//  Serial.begin(9600);
//  while (!Serial) {
//   ; 
//  }
  
  /* 
   * inicjalizowanie karty SD 
   */
    delay(1000);
   while(1)
   {
  if (!SD.begin(53)) {    
    delay(1000);
   // Serial.println("initialization of SD card failed!");
   // delay(500);
  }
    else break;
   }
  
   /* 
    * inicjalizowanie czytnika RFID 
    */
   Serial1.begin(9600);
  if (!Serial1) {
    //Serial.print("Initialization of RFID failed");
    return;
  }
  
  /* 
   * inicjalizowanie modułu GPS 
   */
  Serial2.begin(9600);
  if (!Serial2) {
  //  Serial.print("Initialization of GPS failed");
    return;
  }
  delay(1000);
  
 /* 
  * tworzenie pliku do zapisywania danych zebranych podczas pracy systemy - w petli while, aby uniknąć niestworzenia pliku
  */
  while(1)
  {
  if (SD.exists("data4.txt")) {
   //   Serial.println("data4.txt exists.");
      break;
    } 
    else {
       delay(500);
   //   Serial.println("data4.txt doesn't exist.");
      myFile = SD.open("data4.txt", FILE_WRITE);
      delay(500);
      if (myFile)
      {
        myFile.println("beginning"); 
   //     Serial.println("beginning"); 
        myFile.close();
      }
    }  
  }
delay(500);

/* 
 * petla czekająca na wciśnięcie przycisku startu 
 */
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
}

/* 
 * funkcja LOOP - rozpoczyna pracę po wykonaniu funkcji SETUP i wykonuje się jak pętla while(1) 
 */
void loop()
{
  if (isWorking == true) // jeśli układ ma pracować:
  { 
     /*
      * próba odczytu z modułu RFID - jeśli któryś z 'breloczków jest dostępny, odczytywany jest z niego 14-znakowy kod ID 
      * aby podczas odbierania odpadów nie odczytać kilka razy z rzedu tego samego 'breloczka' wprowadzona zmienną IDcodeprev,
      * która zawiera poprzednio odczytany kod, i jesli jest on równy obecnie odczytanemu, obecnie odczytany kod nie jest 
      * ponownie przetwarzany i zapisywany 
      */
    if (Serial1.available() > 0) 
      { 
         delay (500); 
         IDcodeprev = IDcodeString;
         IDcodeString = "";
        for (int z = 0 ; z < 14 ; z++)
        {   
         IDcodeString+= Serial1.read();
        }
       // Serial.println("ID code:"); 
       // Serial.println(IDcodeString); 
        if (IDcodeString != IDcodeprev )
        {
           /* 
            * prawidłowy odczyt i zapis danych o pojemniku symbolizowany jest mrugnięciem diody led 
            */
           digitalWrite(controlledPin, HIGH);
           delay(700);
           digitalWrite(controlledPin, LOW);
           delay(700);
           digitalWrite(controlledPin, HIGH);
           delay(700);
           digitalWrite(controlledPin, LOW);
           temp_time = 10000;
           RFID_time = millis(); // odnotowanie poprawnego odczytu pojemnika
           saveNewInfo(0); // zapis na kartę SD odpowiednich informacji
        }
      }
      //else 
      //{
        while(Serial2.available())
        {
          /*
           * próba odczytu z modułu GPS, dane przesyłąne są do odpowiednich funkcji klasy GPS i jeśli nastąpi skompletowanie całej sentencji
           * zostaje ona odkodowana i odpowiednie wielkości, takie jak godzina szerokość i długośc geograficzna - zauktalizowane 
           */
          if (my_gps.encode_char(Serial2.read()))
          {
            //Serial.println(my_gps.new_sentence); 
            my_gps.encode();
            counter_error = millis(); // odnotowanie poprawnego odczytu pojemnika
            //gps_debug();
          }
        } 
     // }
      if (checkDistanceSensor()) // jeśli przez odpowiedni czas tylna klapa pojazdu byla otwarta (czyli nastąpoło wyładowanie odpadów)
      {
        saveNewInfo(1); // zapis na kartę SD odpowiednich informacji
      }
      
      /* 
       *  jeśli przez 5 sekund nie nastąpił odczyt czujnika RFID lub minęło 10 sekund od ostatniego zapisu danych 
       *  następuje kolejny zapis  -  informacja o godzinie i połoeniu geograficznym pojazdu 
       */
      if (millis() - RFID_time > temp_time) 
      {
        saveNewInfo(2); // zapis na kartę SD odpowiednich informacji
        temp_time = 5000;
        RFID_time = millis();
      }
      /* 
       *  jeśli minęło 20 sekund lub więcej od ostatniego odczytu sentencji z modułu GPS zmieniamy status urządzenia na 'niepracujący' 
       */
      if (millis() - counter_error > temp_time_error) 
      {
        my_gps.is_working == false;
      }
      /* ustawienie diody led symbolizującej poprawność pracy modułu GPS: świeci - błąd modułu, nie świeci - praca poprawna */
      if (my_gps.is_working  == true)
        digitalWrite(GPScontrolledPin, LOW);
      else
        digitalWrite(GPScontrolledPin, HIGH);

  }
}

 /* 
  * funkcja pomiaru odległości przez czujnik odległości. zwraca wartość w centymetrach 
  */
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

  /* 
   *  funkcja sprawdzająca czy przez 5 sekund wielkość mierzona przez czujnik odległości jest 
   *  większa niż 10cm, jesli jest to prawdą zwraca true, jesli nie - zwraca false
   */
  boolean checkDistanceSensor()
  {
   if (zmierzOdleglosc()>10)
      {
        for (int x = 0 ; x < 10 ; x++)
        {
       //   Serial.println(x);
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
         return true;
      }
      else return false; 
   
  }

  /* 
   *  zapis danych na kartę SD. Mają one rózne formaty w zależności od rodzaju informacji, która ma być w danym momencie zapisana:
   *  gdy parametr i = 0 -> pełna dana zawierająca godzinę, położenie geograficzne, kod ID pojemnika, informację o poprawnym/niepoprawnym odczycie GPS
   *  gdy parametr i = 1 -> dana zawierająca godzinę, położenie geograficzne, informację o tym, iż w tym miejscy wyładowano odpady, informację o poprawnym/niepoprawnym odczycie GPS
   *  inna wartość i -> informacja zawieraca tylko godzine, polożenie geograficzne i informację o poprawności/niepoprawości odczytu GPS
   */
void saveNewInfo( int i)
{
  myFile = SD.open("data4.txt", FILE_WRITE);
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
    if (i == 0) // załadowanie śmieci
    {
      dataString += String(IDcodeString);
      dataString +="#";
    }
    if (i == 1) // informacja o wyładowaniu odpadów
    {
      dataString +="!!!#";
    }
     dataString += String(my_gps.is_working);
     dataString +="#";
  //  Serial.println(dataString);
    myFile.println(dataString);
    myFile.close();
    delay(100);
  } 
  else
   Serial.println("nie wpisuję do pliku"); 
}
/*
 * funkcja wywoływana w trakcie przerwania - eliminuje drgania przycisku STOP 
 * i w razie poprawnego przerwania zmienia status systemu na 'niepracujący'
 */
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
// Serial.println("przetwalem");
}  

/*
 * funkcja do debugowania pracy modułu GPS - wyswietla odebrane dane oraz odkodowane wartości położenia i godziny
 */
void gps_debug()
{
    if (my_gps.loc_updated)
                {
                  dataString = "";
                  dataString += String(my_gps.locNS);
                  dataString +=" ";
                  dataString += String(my_gps.locWE);
                //  Serial.println(dataString);
                }
 
                if (my_gps.time_updated)
                {
                //  Serial.println(my_gps.time_fix);
                }
}

/*
 * funkcja wywołana w SETUP'ie - eliminuje drgania przycisku START 
 * i w razie poprawnego wciśnięcia zmienia status systemu na 'pracujący'
 */
boolean startWorking()
{
  buttonState = digitalRead(startbuttonPin);
  if ( (millis() - lastDebounceTime) > debounceDelay) 
  {
    if ( (buttonState == HIGH) ) 
    {
      digitalWrite(ledPin, HIGH);
      lastDebounceTime = millis(); 
      return true;
    }
    else return false;
  }
  else return false;
}

