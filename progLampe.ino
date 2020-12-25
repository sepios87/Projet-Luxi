#include <Wire.h>
#include "paj7620.h"
#define ActiveShield 4
#define PWMLED 9
#define DIRLed 7 //voir si necessaire
#define PWMMot 10
#define DIRMot 8
#define trig 23
int trig2 = 51; 
int trig3  = 47;
#define echo 25
int echo2 = 53;
int echo3 = 49;
#define pinPhoto A9
#define pinPot A8
#define PIN_LED_R 45
#define PIN_LED_G 43
#define PIN_LED_B 41
#define PIN_LED_R2 27
#define PIN_LED_G2 29
#define PIN_LED_B2 31

byte tableauCouleur[8] = {0b000, 0b100, 0b010, 0b001, 0b101, 0b011, 0b110, 0b111};
byte selecColor = 1;
long lecture_echo, lecture_echo2, lecture_echo3, recTps= 0;
short formuleSensib, sensorValueSensib, luxProdSensib, propoLuxSensib, del, cm, cm2, cm3, cmMoy = 0, valPot, saveCm = 0;
short propoPrecedent = 125, propoLuxPrecedent = 125;
double propoUltrasons, luxDemandeUltrasons;
boolean marche = false;
boolean manu = true;
boolean changeColor = false;
boolean delaiMouvement = false;
long tpsSave = 0;
int aChangerPot = 340;


void setup()
{
      uint8_t error = 0;
  pinMode(ActiveShield, OUTPUT); 
  pinMode(PWMLED, OUTPUT); 
  pinMode(DIRLed,OUTPUT);
  pinMode(DIRMot,OUTPUT);
  digitalWrite(DIRMot,HIGH);
  digitalWrite(DIRLed,HIGH);  
  pinMode(echo, INPUT);
  pinMode(echo2, INPUT);
  pinMode(echo3, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(trig2, OUTPUT);
  pinMode(trig3, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_LED_R2, OUTPUT);
  pinMode(PIN_LED_G2, OUTPUT);
  pinMode(PIN_LED_B2, OUTPUT);
  displayColor(tableauCouleur[7]); //apl methode en bas
      Serial.begin(9600);
    error = paj7620Init();         
}

void loop(){
  uint8_t data = 0, data1 = 0, error;

    error = paj7620ReadReg(0x43, 1, &data);            
    if (!error) 
    {
       if (data == GES_UP_FLAG) {
  paj7620ReadReg(0x43, 1, &data);
  if(!marche){ 
    //si detection mouv,et marche n'etait pas activé passer marche en vrai
    marche = true;
    manu = true;
    analogWrite(PWMMot, 0);
     analogWrite(PWMLED, 160);
    digitalWrite(pinPhoto, HIGH);
    digitalWrite(ActiveShield,HIGH);
    Serial.println("Allumer");
    displayColor2(tableauCouleur[2]);
  } else if(marche){
    //si c t deja en marche, eteindre
    marche = false;
    manu=true;
    changeColor = false;
    analogWrite(PWMLED, 0);
    digitalWrite(pinPhoto, LOW);
    digitalWrite(trig, LOW);
    digitalWrite(trig2, LOW);
    digitalWrite(trig3, LOW);
     displayColor2(tableauCouleur[7]);
    displayColor(tableauCouleur[7]);
    Serial.println("Eteindre");
}
       }
       if (!marche){  
     valPot = analogRead(pinPot);  
      Serial.println(valPot);
     if (valPot < aChangerPot-5){
      analogWrite(PWMMot, 255);
      digitalWrite(DIRMot, LOW); //haut
    }
    if (valPot > aChangerPot){
      analogWrite(PWMMot, 255);
      digitalWrite(DIRMot, HIGH); //bas
      } 
   if (valPot >=aChangerPot-5 && valPot <=aChangerPot){
    digitalWrite(ActiveShield,LOW);
    displayColor(tableauCouleur[7]);
   }
       }
//---------Detection mouvements------
if (marche){
  switch (data) {
    case GES_RIGHT_FLAG:
  paj7620ReadReg(0x43, 1, &data);
  //Droite
 action();
    break;
    case GES_LEFT_FLAG:
  paj7620ReadReg(0x43, 1, &data);
  //Gauche
action();
    break;
    case GES_DOWN_FLAG:
      paj7620ReadReg(0x43, 1, &data);
      //bas
      if (manu && marche){
      delaiMouvement = true;
      if (!changeColor){     
         displayColor2(tableauCouleur[4]);
        Serial.println("Mode changement couleur");
        changeColor = true;
      } else if (changeColor){
         displayColor2(tableauCouleur[2]);
        changeColor = false;  
        Serial.println("Quitte changement couleur");
        }
      }
    break;
  }
  }
    }else if (error){
     Serial.println("erreur!");
      }
  //----------Mode manu------------
if (manu){
  digitalWrite(trig, HIGH);
  delayMicroseconds(500);
  digitalWrite(trig, LOW);
  lecture_echo = pulseIn(echo, HIGH);
  cm = ((lecture_echo * 340 /(10000))/2)-1; // vitesse du son ; conversion millisecondes vers secondes et conversion mètre vers centimètres // le -1 est une marge impressision
  if (cm < 35){
  //40cm max mais declaration 50cm pour etre large
  luxDemandeUltrasons = (800*cm)/45; //application sur 800lux
  propoUltrasons = (255*luxDemandeUltrasons)/800;//essayer de simplifier 
  if(propoPrecedent>propoUltrasons && propoPrecedent<=255)  for (int i = propoPrecedent; i>propoUltrasons; i-=2){ 
    analogWrite(PWMLED, propoPrecedent);
    delay(2);
  }
  if(propoPrecedent<propoUltrasons && propoPrecedent<=255)  for (int i = propoPrecedent; i<propoUltrasons; i+=2){
    analogWrite(PWMLED, propoPrecedent);
    delay(2);
  }
  }
  delay(12);
  propoPrecedent = propoUltrasons;
    }
  //----------Mode auto------------
  if (marche){
  if (!manu) {
 //-----Photoresistance
  sensorValueSensib = analogRead(pinPhoto);
  formuleSensib = pow(sensorValueSensib, 1.75);
  if (800-(formuleSensib*0.056)<800){
    propoLuxSensib = ((800-(formuleSensib*0.056))*255)/800;
  } 
  if (propoLuxSensib<0){
    propoLuxSensib = 30;
  } 
  if(propoLuxPrecedent>propoLuxSensib && propoLuxPrecedent<=255)  for (int i = propoLuxPrecedent; i>propoLuxSensib; i-=2) analogWrite(PWMLED, propoLuxPrecedent);
  if(propoLuxPrecedent<propoLuxSensib && propoLuxPrecedent<=255)  for (int i = propoLuxPrecedent; i<propoLuxSensib; i+=2) analogWrite(PWMLED,propoLuxPrecedent);
  propoLuxPrecedent = propoLuxSensib;
    //Serial.println(sensorValueSensib);
  //---------doubles capteurs
  digitalWrite(trig2, HIGH);
  delayMicroseconds(500);
  digitalWrite(trig2, LOW);
  lecture_echo2 = pulseIn(echo2, HIGH);
  cm2 = ((lecture_echo2 * 340 /(10000))/2)-1; // vitesse du son ; conversion millisecondes vers secondes et conversion mètre vers centimètres // le -1 est une marge impressision
  digitalWrite(trig3, HIGH);
  delayMicroseconds(500);
  digitalWrite(trig3, LOW);
  lecture_echo3 = pulseIn(echo3, HIGH);
  cm3 = ((lecture_echo3 * 340 /(10000))/2)-1; // vitesse du son ; conversion millisecondes vers secondes et conversion mètre vers centimètres // le -1 est une marge impressision
  if (cm2<70 && cm3<70)cmMoy = ((cm2+cm3)/2);
  if (cm2>70 && cm3<70)cmMoy = cm3;
  if (cm2<70 && cm3>70) cmMoy = cm2;
  if (cm2>70 && cm3>70) cmMoy =0;
  valPot = analogRead(pinPot);
 //-----
 if (cmMoy!=0 && tpsSave < millis()){
  if (valPot > aChangerPot-(cmMoy/2)) {
    Serial.println(valPot);
    Serial.println(cmMoy);
         analogWrite(PWMMot, 100);
         digitalWrite(DIRMot, HIGH); //haut
         }
 else if (valPot < aChangerPot-(cmMoy/2)) { //detecte valeur de base a 380
  Serial.println(valPot);
  Serial.println(cmMoy);
   analogWrite(PWMMot, 100);
   digitalWrite(DIRMot, LOW); //bas
         }
  delay(2000);
  tpsSave = millis()+8000;
 }else analogWrite(PWMMot, 0);
 //-----
  if (recTps<millis()){
    recTps= millis()+5000; //delay de 5s 100ms = 1s?
    if (cmMoy<saveCm+2 && cmMoy>saveCm-2 && cmMoy!=0){
      Serial.println("pc");
      Serial.println(cmMoy);
      displayColor(tableauCouleur[4]); //mode pc
    } else if (cmMoy == 0 && saveCm == 0){
       displayColor(tableauCouleur[1]); //mode repos
       Serial.println("repos");
       Serial.println(cmMoy);
    } else{
      displayColor(tableauCouleur[5]); //mode travail
      Serial.println("travail");
      Serial.println(cmMoy);
    }
    saveCm = cmMoy;
      }
    }
  }
}
void action(){
    delaiMouvement = true;
        if (!changeColor){  
          if (manu){
           displayColor2(tableauCouleur[3]);
            manu = false;
            Serial.println("Mode auto");
          } else if (!manu){
            displayColor2(tableauCouleur[2]);
            manu  =true;
            analogWrite(PWMMot, 0);
            digitalWrite(trig2, LOW);
            digitalWrite(trig3, LOW);
            digitalWrite(pinPhoto, LOW);
            Serial.println("Mode manu");
          }
        } else if (changeColor) {
          displayColor2(tableauCouleur[4]);
          selecColor++;
          displayColor(tableauCouleur[selecColor]);
          if (selecColor == 7) selecColor = 0;
        }
}
void displayColor(byte color) { //methode changement de couleurs
  digitalWrite(PIN_LED_R, !bitRead(color, 2));
  digitalWrite(PIN_LED_G, !bitRead(color, 1));
  digitalWrite(PIN_LED_B, !bitRead(color, 0));
}
void displayColor2(byte color) { //methode changement de couleurs
  digitalWrite(PIN_LED_R2, !bitRead(color, 2));
  digitalWrite(PIN_LED_G2, !bitRead(color, 1));
  digitalWrite(PIN_LED_B2, !bitRead(color, 0));
}
