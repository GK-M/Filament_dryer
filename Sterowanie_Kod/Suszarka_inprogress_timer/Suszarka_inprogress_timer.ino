
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
//Biblioteki do DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

#include <PID_v1.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // 16 kolumn, 2 wiersze
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

// Pin danych DS18B20
#define ONE_WIRE_BUS 3
#define PrzyciskEkran 10  // przycisk do GND (aktywny stan niski)
#define FAN_PWM 4         // pin do PWM wentylatora
#define COOK_PWM 2        // pin do PWM mata grzewcza
#define Increase 21       // zwiększanie wartości zadanej
#define Decrease 20       // zmniejszanie wartości zadanej
#define PrzyciskTryb 5    //zmiana trybów 5
#define Enter 1          //Zatwierdzenie 9
#define LedPin 7 //Pin do sterowania pwm
//scl - 9
//sca - 8

// j2 vin
//j3 vout
//czarny sda
//czerwony sdl

// Utworzenie obiektu OneWire i DallasTemperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);



bool backlightOn = true;  // zapamiętany stan podświetlenia
long long CzasDane = 0;
long CoIlePomiar = 1000;    // Co sekunde zbiera dane
int TimeOfflcd = 2 * 1000;  //Czas ile trzymać przycisk
int pTimeOfflcd;
bool LCDflaga = false;    //flaga to zmiany stanu przycisku
bool EnterFlaga = true;  //flaga to zatwierdzania TU MUSI BYC  TRUE
int strona = 1;           //pierwsza strona
int IleStron = 4;         //ile stron do przewijania
int Iletrybow = 3;
long long czasStrona = 0;
const int kana = 0;
const int freq = 1 * 1000;     //1k Hz - PWM dla wentylatora
const int freqCook = 1 * 1000;  //1k Hz - PWM dla grzałki
const int resolution = 10;      //10bit -> zakres duty 0-1023
float temp_aht = 0;
float hum_aht = 0;
bool next = 0;
unsigned long long czasStronap;
int Zadana = 0;
unsigned long long czasTryb = 0;

double tempDS_1 = 0;
double tempDS_2 = 0;
double tempDS_3 = 0;
double DS_2 = 0;
double DS_1 = 0;
double DS_3 = 0;

int tryb = 0;
bool Ptryb = 0;

//Zmienne do timera
uint32_t  SetCzas = 0;

bool FastCzasUp = true;
bool FastCzasDown = true;
unsigned long CzasEnterUp = 0;
unsigned long FastNextCharUp = 0;
unsigned long CzasEnterDown = 0;
unsigned long FastNextCharDown = 0;
const unsigned long debounceDelay = 150;
unsigned long DebounceTimeUp = 0;
unsigned long DebounceTimeDown = 0;
unsigned long ZapisCzasOdliczanie = 0;
uint32_t  StoperCzasMin = 0.0;
uint32_t  StoperCzasGodz = 0.0;
uint32_t  StoperCzasSek = 0.0;
unsigned long doubleStoperCzasGodz = 0.0;
uint32_t  ODLICZ = 0;

bool timerAktywny = false;

//Ekran Wyświetlanie:
uint32_t  SetCzasSek = 0;
uint32_t  SetCzasMin = 0;
uint32_t  SetCzasGodz = 0;

uint32_t czasEnter = 0;
uint32_t dwaczas = 0;

double Setpoint = 0;  // zadana (np. °C)
double Input = 0;     // pomiar
double Output = 0.0;  // wyjście PID (0..100)

// Nastawy (start: PI, bez D)
double Kp = 2.25, Ki = 0.05, Kd = 0.0;

int FAN_MOC = 0;
int RAW = 0;
int COOK_RAW = 0;
int TempGraniczna = 115;

// Led

bool StanLed = HIGH;

//Emotki

uint8_t Stopnie[] = {
  0b11000,
  0b11000,
  0b00111,
  0b01000,
  0b01000,
  0b01000,
  0b01000,
  0b00111
};

uint8_t Dzwonek[] = {
  0b01110,
  0b01010,
  0b01010,
  0b11111,
  0b11111,
  0b00000,
  0b00100,
  0b00000
};
inline void print2d(uint32_t v) {
        if (v < 10) lcd.print('0');
        lcd.print(v);
        }

uint32_t k;
inline void printTemp(uint32_t k) {
  if (k < 10){
    lcd.print("  ");
    }
  else if (k < 99 && k >10){
    lcd.print(" ");   
    }
    lcd.print(k);    
    }

void EnterFlagaa() {
  if (digitalRead(Enter) == LOW && millis()-czasEnter>200 ) {
    czasEnter = millis();
    EnterFlaga = !EnterFlaga;
    timerAktywny =  false;
    ZapisCzasOdliczanie = millis();
    
  }
  
}



  void SETCZAS() {
    // Obsługa Increase
    if (strona == 1 && !EnterFlaga && digitalRead(Increase) == 0 && millis() - DebounceTimeUp > debounceDelay) {
      SetCzas = SetCzas + 900000UL;
      FastCzasUp = false;
      CzasEnterUp = millis();
      FastNextCharUp = CzasEnterUp;
      DebounceTimeUp = millis();
      delay(100);
      Serial.printf("Wartość Zadana to %d\n", SetCzas );
    }

    if (!FastCzasUp && digitalRead(Increase) == 0) {
      if ((millis() - CzasEnterUp > 400) && (millis() - FastNextCharUp > 400)) {
        FastNextCharUp = millis();
        SetCzas = SetCzas + 900000UL;
        Serial.printf("Wartość Zadana to %d\n", SetCzas );
      }
    }

    if (digitalRead(Increase) == 1) FastCzasUp = true;

    // Obsługa Decrease
    if (strona == 1 && !EnterFlaga && digitalRead(Decrease) == 0 && millis() - DebounceTimeDown > debounceDelay) {
      SetCzas = SetCzas - 900000UL;
      FastCzasDown = false;
      CzasEnterDown = millis();
      FastNextCharDown = CzasEnterDown;
      DebounceTimeDown = millis();
      delay(100);
      Serial.printf("Wartość Zadana to %d\n", SetCzas );
    }


    if (!FastCzasDown && digitalRead(Decrease) == 0) {
      if ((millis() - CzasEnterDown > 400) && (millis() - FastNextCharDown > 400)) {
        FastNextCharDown = millis();
        SetCzas = SetCzas - 900000UL;
        Serial.printf("Wartość Zadana to %d\n", SetCzas );
      }
    }
    if (digitalRead(Decrease) == 1) FastCzasDown = true;

    //Zmiana Formatu
    SetCzas = constrain(SetCzas, 0, 86400000);  //max czas do ustawienia 24h

    SetCzasMin = (SetCzas / 60000UL) % 60U;
    SetCzasGodz = (SetCzas / 3600000UL);
  }


  void Odliczanie() {
    if ( EnterFlaga && !timerAktywny && SetCzas!=0) {
      timerAktywny = true;
      ZapisCzasOdliczanie = millis();
     
    }

    if (timerAktywny ) {

      ODLICZ = SetCzas - (millis() - ZapisCzasOdliczanie);
      ODLICZ = constrain(ODLICZ, 0UL, 86400000UL); 
      StoperCzasMin = (ODLICZ / 60000UL) % 60U;
      StoperCzasGodz = (ODLICZ / 3600000UL);
      StoperCzasSek = (ODLICZ / 1000UL) % 60U;
      
      }
    
      if(digitalRead(PrzyciskEkran) == 0 && digitalRead(Enter) == LOW){
        if(millis()-dwaczas>200){
          SetCzas = 0;
          ODLICZ = 0;
          StoperCzasMin = 0;
          StoperCzasGodz = 0;
          StoperCzasSek = 0;
          strona=1;
          dwaczas = millis();
        }
      }
          
    
    
   
      
      
    

    if (StoperCzasMin == 0 & StoperCzasGodz == 0 & StoperCzasSek == 0) {
      Zadana = 0;
      //SetCzas = 0;
      timerAktywny = false;
    }
  }



  void NextStrona() {

    if (digitalRead(PrzyciskEkran) == 0 && millis() - czasStrona > 200 && digitalRead(Enter) != LOW ) {
      czasStrona = millis();
      strona++;
      tryb = 0;
    }
    if (strona > IleStron) strona = 1;
  }
  void NextTryb() {
    if (digitalRead(PrzyciskTryb) == 0 && millis() - czasTryb > 200) {
      czasTryb = millis();
      tryb++;
      strona = 0;
    }
    if (tryb > Iletrybow) tryb = 1;
  }
  void ekranOff() {
    if (digitalRead(PrzyciskEkran) == 0 && !LCDflaga) {
      pTimeOfflcd = millis();
      LCDflaga = true;
    }
    if (digitalRead(PrzyciskEkran) == 1 && LCDflaga) {
      LCDflaga = false;
    }
    if (digitalRead(PrzyciskEkran) == 0 && LCDflaga) {
      if (millis() - pTimeOfflcd > TimeOfflcd) {
        // wciśnięty
        backlightOn = !backlightOn;
        StanLed = !StanLed;

        LCDflaga = false;
        if (backlightOn) {
          lcd.backlight();
          strona = 1;
          digitalWrite(LedPin,HIGH);

        } else {
          lcd.noBacklight();
          digitalWrite(LedPin, LOW);
        }
        
        delay(200);  // proste debounce
      }
    }
  }
  void PomiarZczujnikow() {
    if (millis() - CzasDane > CoIlePomiar) {
      CzasDane = millis();
      
      // AHT20: temp + wilgotność
      sensors_event_t hum, temp;
      aht.getEvent(&hum, &temp);

      temp_aht = temp.temperature;
      hum_aht = hum.relative_humidity;

      // BMP280: temp + ciśnienie
      float t_bmp = bmp.readTemperature();
      float p_hPa = bmp.readPressure() / 100.0f;

      //Temeratura DS18B20

      sensors.requestTemperatures();                 // rozpocznij pomiar
       tempDS_1 = sensors.getTempCByIndex(0);  // odczytaj pierwszy czujnik
       tempDS_2 = sensors.getTempCByIndex(1);  // odczytaj drugi czujnik
       tempDS_3 = sensors.getTempCByIndex(2);  // odczytaj trzeci czujnik

      DS_1 = tempDS_2;
      DS_2 = tempDS_2;
      DS_3 = tempDS_3;

      //Tworzenie Emotek
      lcd.createChar(0, Stopnie);  // Zapisz jako znak nr 0
      lcd.createChar(1, Dzwonek);  // Zapisz jako znak nr 1



      if (strona == 1) {
        
        if(Zadana == 0){
        lcd.setCursor(0, 0);
        lcd.print("T.SET:");
        lcd.print("  0");
        lcd.write(byte(0));  // znak stopni
        lcd.print(" ");
         }
        else if(Zadana<10){
          lcd.setCursor(0, 0);
          lcd.print("T.SET:");
          lcd.print("  ");
          lcd.print(Zadana);
          lcd.setCursor(9, 0);  
          lcd.write(byte(0));  // znak stopni
          lcd.print(" ");
          }
        else if(Zadana>9 && Zadana!=100){
          lcd.setCursor(0, 0);
          lcd.print("T.SET:");
          lcd.print(" ");
          lcd.print(Zadana);
          lcd.setCursor(9, 0);  
          lcd.write(byte(0));  // znak stopni
          lcd.print(" ");
        }
        else {
          lcd.setCursor(0, 0);
          lcd.print("T.SET:");
          lcd.print(Zadana);
          lcd.setCursor(9, 0);  
          lcd.write(byte(0));  // znak stopni
          //lcd.print(" ");
          }
      
        lcd.setCursor(10, 0);
        lcd.print(" ");
        lcd.print((char)126);  // strzałki
        lcd.print(" ");
        if (temp_aht==-127 ){
          lcd.print(" 0");
          lcd.write(byte(0));
        }
        else{
          lcd.print(temp_aht, 1);
          lcd.write(byte(0));  // znak stopni
        }
        //lcd.print(" ");

        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.write(byte(1));  // Znak dzwonka
        lcd.print(" ");
        print2d(SetCzasGodz);
        lcd.print(':');
        print2d(SetCzasMin);
        lcd.print(" ");
        lcd.print((char)126); 
        lcd.print(" ");
        if(StoperCzasGodz==0 && StoperCzasMin==0 && StoperCzasSek == 0){
        lcd.print("00:00:00");
        }
        else{
        print2d(StoperCzasGodz);
        lcd.print(":");
        print2d(StoperCzasMin);
        lcd.print(":");
        print2d(StoperCzasSek);
        }
        //lcd.print(" ");

        lcd.setCursor(0, 2);
        lcd.print("DS1:");
        printTemp(tempDS_1);
        lcd.print((char)223);  // znak stopni
        lcd.print("C");
        //lcd.write(byte(0));
        lcd.print(" ");
        lcd.print("DS3:");
        printTemp(tempDS_3);
        lcd.print((char)223);  // znak stopni
        lcd.print("C");
        //lcd.write(byte(0));
        //lcd.print("H:");
        //lcd.print(hum.relative_humidity, 1);
        lcd.print(" ");
        


        lcd.setCursor(0, 3);
        lcd.print("Humidity: ");
        lcd.print(hum.relative_humidity, 1);
        lcd.print("%    ");
       
       
    
      }

      if (strona == 2) {
        
        lcd.setCursor(0, 0);
        lcd.print("Dane z czujnikow:   ");

        lcd.setCursor(0, 1);
        lcd.print("T_aht:");
        lcd.print(temp_aht, 1);
        lcd.write(byte(0));  // znak stopni
        lcd.print(" H:");
        lcd.print(hum.relative_humidity, 1);lcd.print("% "); 

        lcd.setCursor(0, 2);
        lcd.print("T_bmp:");
        lcd.print(t_bmp, 1);
        lcd.write(byte(0));  // znak stopni
        lcd.print(" P:");
        lcd.print(p_hPa, 0);
        lcd.print("hPa");

        lcd.setCursor(0, 3);
        lcd.print("T1");
        printTemp(tempDS_1);
        lcd.write(byte(0));
        lcd.print(" T2");
        printTemp(tempDS_2);
        lcd.write(byte(0));
        lcd.print(" T3");
        printTemp(tempDS_3);
        lcd.write(byte(0));
      }
      if (strona == 3) {
        // Linia 2: ciśnienie (z BMP280) + temp z BMP
        lcd.setCursor(0, 0);
        lcd.print("    Regulator PID   ");
        lcd.setCursor(0, 1);
        lcd.print(" Setpoint: ");
        lcd.print(Setpoint,0);
        lcd.print("       ");
        lcd.setCursor(0, 2);
        lcd.print(" Input: ");
        lcd.print(Input,0);
        lcd.print("          ");
        lcd.setCursor(0, 3);
        lcd.print(" Output: ");
        lcd.print(Output,0);
        lcd.print("          ");
        
      }
      if (strona == 4) {
        lcd.setCursor(0, 0);
        lcd.print("        PWM         ");
        lcd.setCursor(0, 1);  
        lcd.print(" FAN: ");
        lcd.print(FAN_MOC);
        lcd.print("%           ");  // znak stopni
        lcd.setCursor(0, 2);  
        lcd.print(" FAN_PWM:  ");
        lcd.print(RAW);
        lcd.print("      ");
        lcd.setCursor(0, 3);
        lcd.print(" COOK_PWM:  ");
        lcd.print(COOK_RAW);
        lcd.print("       ");

      }
      if (tryb == 1) {
        lcd.setCursor(0, 0);
        lcd.printf("Tryb: %d               ", tryb);
        lcd.setCursor(0, 1);
        lcd.print("      DRYING          ");
        Serial.printf("TRYB: %d\n", tryb);
      }
      if (tryb == 2) {
        lcd.setCursor(0, 0);
        lcd.printf("Tryb: %d           ", tryb);
        lcd.setCursor(0, 1);
        lcd.print("      MAINT        ");
        Serial.printf("TRYB: %d\n", tryb);
      }
      if (tryb == 3) {
        lcd.setCursor(0, 0);
        lcd.printf("Tryb: %d                 ", tryb);
        lcd.setCursor(0, 1);
        lcd.print("     ECO SLEEP      ");
        Serial.printf("TRYB: %d\n", tryb);
      }
    }
  }
  
  void setPWMpercentCOOK(int bits, int pct) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    // prog rozruchu (duzo 12V wentylatorow nie rusza <30-40%)
    if (pct > 0 && pct < 20) pct = 20;

    int maxv = (1 << bits) - 1;           // np. 1023 dla 10 bit
    int raw = map(pct, 0, 100, 0, maxv);  // % -> raw
    analogWrite(COOK_PWM, raw);           // wyjscie na pin
    COOK_RAW=raw;
  }
  
  void setPWMpercent(int bits, int pct) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    // prog rozruchu (duzo 12V wentylatorow nie rusza <30-40%)
    if (pct > 0 && pct < 20) pct = 20;

    int maxv = (1 << bits) - 1;           // np. 1023 dla 10 bit
    int raw = map(pct, 0, 100, 0, maxv);  // % -> raw
    analogWrite(FAN_PWM, raw);            // wyjscie na pin
    RAW=raw;
  }
  
  void Logika_Wentylator() {

    if ((hum_aht > 50 && hum_aht < 100) && Zadana == temp_aht ) FAN_MOC = 100;
    else if (hum_aht > 40 && hum_aht <= 50 && Zadana == temp_aht) FAN_MOC = 80;
    else if (hum_aht > 30 && hum_aht <= 40 && Zadana == temp_aht) FAN_MOC = 70;
    else if (hum_aht > 0 && hum_aht <= 20 && Zadana == temp_aht) FAN_MOC = 60;
    else FAN_MOC = 100;  // domyslny
    setPWMpercent(resolution, FAN_MOC);

  }

  const uint16_t debounceMs = 50;
  long long CzasINC = 0;
  long long CzasZadana = 0;
  long long CzasPress = 0;
  bool ZadanaFlaga = false;
  bool pStaninc = 1;
  bool StanInc = digitalRead(Increase);


  void TempZadanaUp() {

    if (digitalRead(Increase) == 0 && !ZadanaFlaga && EnterFlaga && strona == 1  ){ 
      CzasZadana = millis();
      CzasPress = CzasZadana;
      ZadanaFlaga = true;
      Zadana = Zadana + 5;
      Zadana = constrain(Zadana, 0, 100);
      Serial.printf("Wartość Zadana to %d\n", Zadana);
      delay(100);
      
    }
    else if (digitalRead(Increase) == 0 && ZadanaFlaga) {
      if (millis() - CzasZadana > 400 && millis() - CzasPress > 200) {
        CzasPress = millis();
        Zadana = Zadana + 5;
        Zadana = constrain(Zadana, 0, 100);
        Serial.printf("Wartość Zadana to %d\n", Zadana);
      }
    }
    if (digitalRead(Increase) == 1 && ZadanaFlaga) {
      ZadanaFlaga = false;
    }
  }
  unsigned long CzasZadanaUp = 0;
  unsigned long CzasPressUp = 0;
  bool ZadanaFlagaUp = false;

  void TempZadanaDown() {

    if ((digitalRead(Decrease) == LOW) && !ZadanaFlagaUp && EnterFlaga && strona == 1 ) { //
      CzasZadanaUp = millis();
      CzasPressUp = CzasZadanaUp;
      ZadanaFlagaUp = true;
      Zadana = Zadana - 5;
      Zadana = constrain(Zadana, 0, 100);
      delay(100);
      Serial.printf("Wartość Zadana to %d\n", Zadana);
    }

    else if ((digitalRead(Decrease) == LOW) && ZadanaFlagaUp) {
      if ((millis() - CzasZadanaUp > 400) && (millis() - CzasPressUp > 200)) {
        CzasPressUp = millis();
        Zadana = Zadana - 5;
        Zadana = constrain(Zadana, 0, 100);
        Serial.printf("Wartość Zadana to %d\n", Zadana);
      }
    }

    if ((digitalRead(Decrease) == HIGH) && ZadanaFlagaUp) {
      ZadanaFlagaUp = false;
    }
  }



  PID pid(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

  void COOK() {
     if(tempDS_1 >= TempGraniczna || tempDS_2 >= TempGraniczna || tempDS_3 >= TempGraniczna){
      Zadana = 0;
      strona = 0;
      Setpoint = 0;
      tryb = 0;
      
    lcd.setCursor(0, 1);
    lcd.print("Setpoint: ");
    lcd.print(Setpoint);
    lcd.print("      ");

    lcd.setCursor(0, 2);
    lcd.print("DS1: ");
    lcd.print(tempDS_1, 1);
    lcd.write(byte(0));
    lcd.print("     ");

    lcd.setCursor(0, 3);
    lcd.print("DS3: ");
    lcd.print(tempDS_3, 1);
    lcd.write(byte(0));
    lcd.print("     ");
    delay(1000*60*5);
  }
    
    else {

    Setpoint = Zadana;
    Input = temp_aht;   // ← podmień na swój czujnik (np. temperatura)
    pid.Compute();  // liczy co SetSampleTime ms; wynik w Output (0..100)
    setPWMpercentCOOK(resolution, (int)Output); 
    }
    
    
    
    
  }


  void setup() {
    Serial.begin(119200);
    delay(1000);

    Wire.begin(8, 9);  // I2C do Ekranu/AHT20/BMP280
    Wire.setClock(400000); //odświeżanie I2C
    lcd.init();       // Włącz ekran
    lcd.backlight();  // Ekran ON

    pinMode(PrzyciskEkran, INPUT_PULLUP);  // wewnętrzny pull-up dla przycisku
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP);   // pull-up pod onewire
    pinMode(Increase, INPUT_PULLUP);       // plus wartość zadana
    pinMode(Decrease, INPUT_PULLUP);       // minus wartość zadana
    pinMode(PrzyciskTryb, INPUT_PULLUP);   // przycisk do trybów
    pinMode(Enter, INPUT_PULLUP);          //Przycisk od zatwierdzania
    //LED
    analogWriteResolution(LedPin, resolution);  // 10-bit -> 0..1023
    analogWriteFrequency(LedPin, freq);
    pinMode(LedPin,OUTPUT);                //PWM do ledów
    digitalWrite(LedPin,HIGH);

    sensors.begin();  // start magistrali OneWire
    sensors.setWaitForConversion(false); // tryb nieblokujący (płynna pętla)
    
    //Ustawienie PWM
    analogWriteResolution(FAN_PWM, resolution);  // 10-bit -> 0..1023
    analogWriteFrequency(FAN_PWM, freq);

    //Ustawienie PWM grzałki
    analogWriteResolution(COOK_PWM, resolution);  // 10-bit -> 0..1023
    analogWriteFrequency(COOK_PWM, freqCook);

    
    
                                  // PID
    pid.SetOutputLimits(0, 100);  // WYJŚCIE 0..100
    pid.SetSampleTime(500);       // ms; 
    pid.SetMode(AUTOMATIC);
  }

  void loop() {
   
   
      
    EnterFlagaa();
    TempZadanaDown();
    TempZadanaUp();
    SETCZAS();
    Odliczanie();
    NextStrona();
    NextTryb();
    PomiarZczujnikow();
    ekranOff();
    Logika_Wentylator();
    COOK();
    
    

  }
