# Arduino project that collects data during the route of a garbage truck.

PL:

Niniejsze repozytorium zawiera oprogramowanie systemu służącego do monitorowania gospodarki odpadami komunalnymi, czyli wyposażenia pojazdu transportującego odpady w system wykorzystujący GPS, rejestrujący momenty załadunku i wyładunku odpadów oraz umożliwiający trwałe zapisywanie i przechowywanie danych. Głównym elementem układu jest moduł Arduino. Do niego podłączono szereg czujników zbierających różnorodne parametry (położenie geograficzne, informacje o obsłużonych pojemnikach i otwarciach tylnej klapy samochodu), które finalnie zapisywane są na karcie SD w określony sposób, ułatwiający ich późniejszą interpretację. Po przebytym kursie dane trafiajądo komputera, gdzie za pomocą oprogramowania zostaną przetworzone i wyświetlone (oprogramowanie to jest zamieszczone w repozytorium 'google_maps').

W projekcie wykorzystano nasjępujące elementy:
- Arduino MEGA 2560 - jaki moduł główny,
- moduł Grove 125kHz RFID reader- odczytuje od adresy opróżnianych pojemników na śmieci (na każdym koszu umieszczony jest brelok RFID),
- moduł GPS oparty na układzie SIM28 - zbiera informacje na temat położenia geograficznego samochodu ( do obsługi tego elementu napisano własną bibliotekę zawartą w folderze 'GPS_lib'),
-  ultradźwiękowy czujnik odległości HC-SR04 - wyłapuje on momenty otwierania tylnej klapy pojazdu co oznacza wyładowanie zgromadzonych w pojeździe odpadów,
- płytka kontrolna wykonana ręcznie - zawiera przyciski do uruchomienia/zatrzymania pracy systemu oraz diody informujące o stanie pracy systemu (tryb gotowości do pobierania danych, braku zasięgu GPS oraz zwizualizowania poprawnego odczytu breloka RFID),
- czytnik kart SD obsługiwany za pomocą interfejsu SPI - na karcie zapisywane są dane, które następnie przenoszone są do komputera i wyświetlane za pomocą aplikacji.


W systemie wykorzystano następujące piny Płytki Arduino MEGA 2560:

- pin cyfrowy numer 2 - pin przycisku zaczynającego pracę (START), 
- pin cyfrowy numer 3 - pin przycisku kończącego pracę (STOP), 
- pin cyfrowy numer 4 - pin diody zaczynającej świecić przy uruchomieniu systemu, 
- pin cyfrowy numer 5 - pin diody migającej przy odebraniu/wyładowani odpadów, 
- pin cyfrowy numer 6 - pin diody świecącej w przypadku niedziałania modułu GPS, 
- pin cyfrowy numer 12 - pin "TRIG" czujnika odległości, 
- pin cyfrowy numer 13 - pin "ECHO" czujnika odległości, 
- pin numer 16 (TX2) - pin TX modułu GPS, 
- pin numer 17 (RX2) - pin RX modułu GPS, 
- pin numer 18 (TX1) - pin TX modułu RFID, 
- pin numer 19 (RX1) -  pin RX modułu RFID, 
- pin cyfrowy numer 50 - port MISO na czytniku kart SD,
- pin cyfrowy numer 51 - port MOSI na czytniku kart SD, 
- pin cyfrowy numer 52 - port SCK na czytniku kart SD, 
- pin cyfrowy numer 53 - port SS na czytniku kart SD, 
- masa układu - piny GND każdego z urządzeń (także masy diod i przycisków), 
- zasilanie z pinu 5 V - zasilanie wszystkich komponentów poza modułem GPS, 
- zasilanie z pinu 3,3 V - zasilanie modułu GPS.
 

Program zawiera bardzo dokładne komentarze, także zrozumienie pracy systemu nie powinno stanowić dla nikogo problemu.

