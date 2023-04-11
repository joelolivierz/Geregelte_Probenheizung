#include "Arduino.h"
#define BUZZER 27
int time1;

void initBuzzer(){
  pinMode(BUZZER, OUTPUT);
}

//function waits overall 600ms
void buzzerSoundRegulating(){
  for(time1=0;time1<1000;time1++){
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(100);
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(100);
  }
  delay(200);
  for(time1=0;time1<1000;time1++){
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(100);
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(100);
  }
}

//function waits overall 10ms
void buzzerSoundDisplay(){
  for(time1=0;time1<10;time1++){
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(500);
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(500);
  }
}