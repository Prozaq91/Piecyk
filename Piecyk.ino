/*
 * 0 - stop
 * 1 - rozruch start swieca 500ms
 * 2 - rozruch start wentylator 3s (wydatek wentylatora niski)
 * 3 - rozruch start podawania paliwa 60s (niski wydatek),wylaczenie swiecy po (60 s + detekcji plomienia)
 * 4 - rozruch wylaczenie swiecy po (60 s + detekcji plomienia)
 * 5 - rozruch start wentylator i pompa po 100 s (sredni wydatek)
 * 6 - normalna praca wentylator i pompa po 260 s (wysoki wydatek)
 * 7 - zatrzymywanie wylaczenie paliwa
 * 8 - zatrzymywanie start swieca 
 * 9 - zatrzymywanie wylaczanie swiecy
 * 10 - zatrzymanie wylaczenie wentylatora
 */

// ******* biblioteki ************
#include <LiquidCrystal.h>
#include <OneWire.h>

// **** wejscia wyjscia **********
#define setpoint_key_up 8
#define setpoint_key_dwn 9
#define setpoint_key_ok 10
#define drv_swieca 13
#define drv_pompa 12
#define drv_wentylator 11

// ********* zmienne *************
int status_pracy = 0;           // 0 - stop, 1 - rozruch, 2 - normalna praca, 3 - zatrzymywanie
int zaplon = 0;
int wentylator = 0;
int pompa_paliwa = 0;         

float setpoint = 21.0;
float Tzew = 15.0;
float dTSTART = 5.0;
float Tuchyb = 0.0;

const int Pompa_Imp_Time = 100;
const int Pompa_Low_Rate = 600;
const int Pompa_Mid_Rate = 400;
const int Pompa_Max_Rate = 250;

unsigned long time_to_show_data = 0;
unsigned long time_to_get_data = 0;
unsigned long sequece_timer = 0;

// ********* funkcje ***********
void lcd(){
  Serial.print("SetPoint: ");
  Serial.println(setpoint);
  Serial.print("Status Pracy: ");
  Serial.println(status_pracy);
 
}

void get_data(){
  Tzew = 11.4;  
  Tuchyb = setpoint - Tzew; //24 - 19 = 5
}

void setpoint_plus(){
  setpoint = setpoint + 0.5;
  lcd(); 
}

void setpoint_minus(){
  setpoint = setpoint - 0.5;  
  lcd();
}

void sterowanie_swieca(){
  if(zaplon == 1){
    digitalWrite(drv_swieca,HIGH);
  }else{
    digitalWrite(drv_swieca,LOW);
  }
 }

void sterowanie_wentylator(){ 
  if(wentylator == 1){
    digitalWrite(drv_wentylator,HIGH);
  }else{
    digitalWrite(drv_wentylator,LOW);
  }
 }

 void sterowanie_pompa(){
  if(pompa_paliwa == 1 && status_pracy == 2){
    delay(Pompa_Imp_Time);
    digitalWrite(drv_pompa,HIGH);
     delay(Pompa_Low_Rate);
    digitalWrite(drv_pompa,LOW);
   
  }
 

if(pompa_paliwa == 1 && status_pracy == 3){
    delay(Pompa_Imp_Time);
    digitalWrite(drv_pompa,HIGH);
     delay(Pompa_Mid_Rate);
    digitalWrite(drv_pompa,LOW);
  
 
  }
 
 if(pompa_paliwa == 1 && status_pracy == 4){
    delay(Pompa_Imp_Time);
    digitalWrite(drv_pompa,HIGH);
     delay(Pompa_Max_Rate);
    digitalWrite(drv_pompa,LOW);
  
 
  }
 }
// ************************************** //
void setup(){
pinMode(drv_swieca,OUTPUT);
pinMode(drv_pompa,OUTPUT);
pinMode(drv_wentylator,OUTPUT);
pinMode(setpoint_key_up,INPUT_PULLUP);
pinMode(setpoint_key_dwn,INPUT_PULLUP);
pinMode(setpoint_key_ok,INPUT_PULLUP);
digitalWrite(drv_swieca,LOW);
digitalWrite(drv_pompa,LOW);
digitalWrite(drv_wentylator,LOW);
Serial.begin(115200);

lcd();
}


void loop(){
// odczyt temperatury
if(millis() >= time_to_get_data){
  get_data();
  time_to_get_data = millis() + 1000;
}

// sekwencja sterowania zmieniamy tylko status pracy
if(status_pracy == 0 && Tuchyb >= dTSTART){             // ustawiamy sekwencje na 1 (zapłon)
  status_pracy = 1; 
  sequece_timer = millis() + 60000;
}

if(status_pracy == 1 && millis() >= sequece_timer){    // ustawiamy sekwencje na 2 (wentylator + pompa) MIN
  status_pracy = 2;
  sequece_timer = millis() + 100000;
}

if(status_pracy == 2 && millis() >= sequece_timer){    // ustawiamy sekwencje na 3 (wentylator + pompa) MID
  status_pracy = 3; 
  sequece_timer = millis() + 260000;

}

if(status_pracy == 3 && millis() >= sequece_timer){    // ustawiamy sekwencje na 4 (wentylator + pompa) MAX
  status_pracy = 4;
}

// sterowanie peryferiami na podstawie statusu pracy
switch(status_pracy){
  case 0:               // stop
    zaplon = 0;
    wentylator = 0;
    pompa_paliwa = 0;
  break;
  case 1:               //rozruch (swieca) + (wentylator MIN)
    zaplon = 1;
    wentylator = 1;
    pompa_paliwa = 0;
  break;
  case 2:               //rozruch (pompa paliwa MIN) + (wentylator MIN)
    zaplon = 0;
    wentylator = 1;
    pompa_paliwa = 1;
  break;  
  case 3:               //rozruch (pompa paliwa MID) + (wentylator MID)
    zaplon = 0;
    wentylator = 1;
    pompa_paliwa = 1;
  break;
  case 4:               //rozruch (pompa paliwa MAX) + (wentylator MAX)
    zaplon = 0;
    wentylator = 1;
    pompa_paliwa = 1;
    
  break;
 
   }


  // obsluga przyciskow
  if(digitalRead(setpoint_key_up) == LOW){      //sprawdzanie czy przycisk jest wcisnięty
   delay(100);                                  //odczekajmy 100msec
    if(digitalRead(setpoint_key_up) == LOW){    // jesli nadal wcisniety
        setpoint_plus();                        // odpalamy dodanie 0.5C
        delay(300);                             //zwłoka po zmianie kroku 300msec
      }
    }
    
  if(digitalRead(setpoint_key_dwn) == LOW){      //sprawdzanie czy przycisk jest wcisnięty
   delay(100);                                  //odczekajmy 100msec
    if(digitalRead(setpoint_key_dwn) == LOW){    // jesli nadal wcisniety
        setpoint_minus();                        // odpalamy dodanie 0.5C
        delay(300);                             //zwłoka po zmianie kroku 300msec
    }
  }    

  
  // automatyczne odswiezanie LCD 
  if(millis() >= time_to_show_data){
    lcd();
    time_to_show_data = millis() + 10000;
  }

sterowanie_swieca();
sterowanie_wentylator();
sterowanie_pompa();
}
