//includes for headers
#include "Display.h"
#include "MCP3426.h"
#include "TouchScreen.h"
#include "Buzzer.h"

//for temperature calculation
//numbers were obtained from the Pt1000-calibration 
#define A 0.003914323
#define B -0.00000064641353
#define RNULL 1000.2100031
#define R 1000
#define UEIN 3.3

//defines to update temps on the screen
#define PRINTSHOULDTEMP 0
#define PRINTISTEMP 1

//defines for state-machine
#define EVALUATE 0                          //state to decide in which mode the heating cartridge goes
#define HEATING 1       
#define SOFTHEATING 2                       //if isTemp and shouldTemp are close to each other
#define COOLING 3
#define REGULATING 4                        //to hold shouldTemp
#define STANDBY 5

//For off-temp of the heating cartridge
#define AKONST -0.0002
#define BKONST 0.9989
#define CKONST 5.0465

//output pin
#define HEATINGCARTRIDGE 33

//for status lamp
#define RED 0
#define GREEN 1

//touchscreen
#define YP A5
#define XM A1
#define YM 15
#define XP 32
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 559); //last argument is resistance across the X plate

//help variables
float shouldTemp = 50, isTemp, stoppHeatingTemp;
int status = STANDBY, PWMCounter = 0, PWMPowerChoice = 1;
bool toggle = false;

void setup() {
  Serial.begin(115200);
  initMCP3426();
  loadGUI();
  initBuzzer();
  updateStoppHeatingTemp();
  updateTemps(shouldTemp, PRINTSHOULDTEMP);
  pinMode(HEATINGCARTRIDGE, OUTPUT);
  digitalWrite(HEATINGCARTRIDGE, HIGH);
}

float getTemp(){
  long ADCvalue;
  float voltage, Umeas, MNFcFactor, unadjustedTemp;
  ADCvalue = readMCP3426();
  voltage = (float)ADCvalue*0.0000625;
  Umeas = voltage / 7;
  MNFcFactor = 1 - (R*(Umeas+((R*UEIN)/(R+R))))/(RNULL*(UEIN-Umeas-((R*UEIN)/(R+R))));
  unadjustedTemp = ((-A+(float)sqrt(A*A-4*B*MNFcFactor))/(2*B));
  return (unadjustedTemp-((-0.0079*unadjustedTemp)+0.8069));                                    //adjusting temp with correction function obtained from system-calibration
}

//function to calculate the stopp-heating-temp so that the overshoot lands on the shouldTemp
void updateStoppHeatingTemp(){
  stoppHeatingTemp = (-BKONST+(float)sqrt(BKONST*BKONST-4*AKONST*(CKONST-shouldTemp)))/(2*AKONST);
}

void loop() {
  isTemp = getTemp();
  updateTemps(isTemp, PRINTISTEMP);

  TSPoint p = ts.getPoint();
  //scan for up/down button
  if((p.y>-1150)&&(p.z<-800)){
    if((p.x>-1050)&&(p.x<-620)){
      if(shouldTemp>20){
        shouldTemp--;
        updateStoppHeatingTemp();
        updateStatusLamp(RED);
        updateTemps(shouldTemp, PRINTSHOULDTEMP);
        buzzerSoundDisplay();
        if(toggle){
          status = EVALUATE;
        }
      }
    }
    if((p.x>-1750)&&(p.x<-1300)){
      if(shouldTemp<100){
        shouldTemp++;
        updateStoppHeatingTemp();
        updateStatusLamp(RED);
        updateTemps(shouldTemp, PRINTSHOULDTEMP);
        buzzerSoundDisplay();
        if(toggle){
          status = EVALUATE;
        }
      }
    }
  }
  
  //scan for start/stopp button
  if((p.x>180)&&(p.x<470)){
    if((p.y<350)&&(p.y>-290)){
      toggle = !toggle;
      buzzerSoundDisplay();
      if(toggle){
        status = EVALUATE;
      }else{
        status = STANDBY;
        updateStatusLamp(RED);
      }
      updateStartStoppButton(toggle);
    }
  }

  switch(status){
    case EVALUATE:      if((stoppHeatingTemp-isTemp)>=8){
                          digitalWrite(HEATINGCARTRIDGE, LOW);
                          status = HEATING;
                        }
                        if((isTemp<shouldTemp)&&((stoppHeatingTemp-isTemp)<8)){
                          status = SOFTHEATING;
                        }
                        if(isTemp>shouldTemp){
                          digitalWrite(HEATINGCARTRIDGE, HIGH);
                          status = COOLING;
                        }
                        if(((shouldTemp-isTemp)>=-0.4)&&(shouldTemp-isTemp)<=1.4){
                          status = REGULATING;
                          updateStatusLamp(GREEN);
                        }
    break;
    case HEATING:       if(isTemp>=stoppHeatingTemp){
                          digitalWrite(HEATINGCARTRIDGE, HIGH);
                        }
                        if(isTemp>=(shouldTemp-0.5)){                       //caused of possible tolerances between the different heating cartridges, here minus 0.5K
                          status = REGULATING;
                          updateStatusLamp(GREEN);
                          buzzerSoundRegulating();
                        }
    break;
    case SOFTHEATING:   PWMCounter++;
                        if(PWMCounter<=2){                                //20% of possible power
                          digitalWrite(HEATINGCARTRIDGE, LOW);
                        }else{
                          digitalWrite(HEATINGCARTRIDGE, HIGH);
                          if(PWMCounter>=10){
                            PWMCounter = 0;
                          }
                        }
                        if(isTemp>=(shouldTemp-0.8)){
                          status = REGULATING;
                          updateStatusLamp(GREEN);
                          buzzerSoundRegulating();
                        }
    break;
    case COOLING:       if(isTemp<=shouldTemp){
                          status = REGULATING;
                          updateStatusLamp(GREEN);
                          buzzerSoundRegulating();
                        }
    break;
    case REGULATING:    if(shouldTemp>80){
                          PWMPowerChoice = 2;
                        }else{
                          PWMPowerChoice = 1;
                        }
                        digitalWrite(HEATINGCARTRIDGE, HIGH);               //security measure that the heating cartridge doesn't stay high if the PWM ends in a LOW
                        if(isTemp<=(shouldTemp-0.05)){                  
                          PWMCounter++;
                          if(PWMCounter<=PWMPowerChoice){                   //10% or 20% of possible power
                            digitalWrite(HEATINGCARTRIDGE, LOW);
                          }else{
                            digitalWrite(HEATINGCARTRIDGE, HIGH);
                            if(PWMCounter>=10){
                              PWMCounter = 0;
                            }
                          }
                        }
    break;
    case STANDBY:       digitalWrite(HEATINGCARTRIDGE, HIGH);
    break;
  }
}