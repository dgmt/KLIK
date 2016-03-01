//By David Szebenyi 2016

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//INCLUDE EEPROMEX LIBRARY

#include <EEPROMex.h>
#include <EEPROMVar.h>

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//SET UP SHIFT REGISTERS FOR LEDS

int SER_Pin = 8;   //pin 14 on the 75HC595
int RCLK_Pin = 10;  //pin 12 on the 75HC595
int SRCLK_Pin = 9; //pin 11 on the 75HC595

//How many of the shift registers - change this
#define number_of_74hc595s 2  

//do not touch
#define numOfRegisterPins number_of_74hc595s * 8

boolean registers[numOfRegisterPins];

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = LOW;
  }
} 


//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){

  digitalWrite(RCLK_Pin, LOW);

  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(SRCLK_Pin, LOW);

    int val = registers[i];

    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);

  }
  digitalWrite(RCLK_Pin, HIGH);

}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//ENCODER VARS
int encoderPin1 = 2;
int encoderPin2 = 3;

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

//SAVE ADDRESS
int address = 0;

//TIMER VARS
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long pMillis = 0;

bool blinky;

//STEPS ARRAY
boolean steps[16];

//CLOCK IN
bool clkin;
bool pclk;

bool changedir;

//DEFAULT VALUES
int rstep=0;
int curs=0;
int lastcurs=0;
int playhead=0;
int lastStep = 15;

//SAVE CURSOR POSITIONS
int saveCurs=0;
int saveMenuCurs=0;
int saveLastStepCurs=0;

//RESET
bool resIn=false;
bool pRes=false;

//MENU
bool menuItem[4]{1,0,0,0};
bool playModeMenu=false;
bool lastStepMenu=false;

unsigned long buttonTimer=0;

bool buttonActive=false;
bool longPressActive=false;
bool lastStepActive=false;
bool randomActive=false;
bool resetActive=false;

//MENU TIMES
int playModeTime = 1000;
int lastStepTime=2000;
int randomTime=3000;
int resetTime=4000;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//EEPROM SAVE SETUP

struct saveSteps {
bool value;
bool playmode;
int laststep;
};

saveSteps saveStepsInput[21];
saveSteps saveStepsOutput[21];

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void setup() {
  Serial.begin (9600);

  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);

  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);

  pinMode(4, INPUT); //button
  pinMode(5, INPUT); //clk in
  pinMode(6, INPUT); //res in
  pinMode(11, OUTPUT); //out

  //reset all register pins
  clearRegisters();
  
  //WELCOME ANIMATION
  setRegisterPin(0,HIGH);
  setRegisterPin(1,LOW);
  setRegisterPin(2,LOW);
  setRegisterPin(3,LOW);
  setRegisterPin(4,LOW);
  setRegisterPin(5,LOW);
  setRegisterPin(6,LOW);
  setRegisterPin(7,LOW);
  setRegisterPin(8,LOW);
  setRegisterPin(9,LOW);
  setRegisterPin(10,LOW);
  setRegisterPin(11,LOW);
  setRegisterPin(12,LOW);
  setRegisterPin(13,LOW);
  setRegisterPin(14,LOW);
  setRegisterPin(15,LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(0,LOW);
  setRegisterPin(1,LOW);
  setRegisterPin(2,LOW);
  setRegisterPin(3,LOW);
  setRegisterPin(4,LOW);
  setRegisterPin(5,HIGH);
  setRegisterPin(6,HIGH);
  setRegisterPin(7,LOW);
  setRegisterPin(8,LOW);
  setRegisterPin(9,HIGH);
  setRegisterPin(10,HIGH);
  setRegisterPin(11,LOW);
  setRegisterPin(12,LOW);
  setRegisterPin(13,LOW);
  setRegisterPin(14,LOW);
  setRegisterPin(15,LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(0,LOW);
  setRegisterPin(1,HIGH);
  setRegisterPin(2,HIGH);
  setRegisterPin(3,LOW);
  setRegisterPin(4,HIGH);
  setRegisterPin(5,LOW);
  setRegisterPin(6,LOW);
  setRegisterPin(7,HIGH);
  setRegisterPin(8,HIGH);
  setRegisterPin(9,LOW);
  setRegisterPin(10,LOW);
  setRegisterPin(11,HIGH);
  setRegisterPin(12,LOW);
  setRegisterPin(13,HIGH);
  setRegisterPin(14,HIGH);
  setRegisterPin(15,LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(0,LOW);
  setRegisterPin(1,LOW);
  setRegisterPin(2,LOW);
  setRegisterPin(3,LOW);
  setRegisterPin(4,LOW);
  setRegisterPin(5,LOW);
  setRegisterPin(6,LOW);
  setRegisterPin(7,LOW);
  setRegisterPin(8,LOW);
  setRegisterPin(9,LOW);
  setRegisterPin(10,LOW);
  setRegisterPin(11,LOW);
  setRegisterPin(12,LOW);
  setRegisterPin(13,LOW);
  setRegisterPin(14,LOW);
  setRegisterPin(15,LOW);
  writeRegisters();
  delay(100);
  
  //LOAD LAST SAVED STATE
  EEPROM.readBlock(address, saveStepsOutput,21);
  
  for (int i=0;i<16;i++){
    steps[i]=saveStepsOutput[i].value;
  }
  
  for (int i=0;i<4;i++){
    menuItem[i]=saveStepsOutput[i+16].playmode;
  }

  lastStep=saveStepsOutput[20].laststep;
  
  updateSteps();
  
  setRegisterPin(curs, HIGH);
  
  //GENERATE NEW RANDOM SEED
  randomSeed(analogRead(A0));
  
  writeRegisters();
  
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//UPDATE LEDS ACCORDING TO WHAT'S HAPPENING
void updateSteps() {
if (!playModeMenu && !longPressActive && !lastStepMenu && !lastStepActive) {
for (int i=0; i <= lastStep; i++){
      if (steps[curs]==0) {
        if (currentMillis - previousMillis >= 80) {
          if (blinky==0) {
          setRegisterPin(curs, HIGH);
          blinky=1;
          } else {
          setRegisterPin(curs, LOW);
          blinky=0;
          }
          previousMillis = currentMillis;
        }
        } else {
          if (currentMillis - previousMillis >= 20) {
            if (blinky==0) {
            setRegisterPin(curs, HIGH);
            blinky=1;
            } else {
            setRegisterPin(curs, LOW);
            blinky=0;
            }
          previousMillis = currentMillis;
        }
      }
    if (steps[i]==1 && (i != curs && i != playhead)) {
        setRegisterPin(i, HIGH);
      }

    if (steps[i]==0 && (i != curs && i != playhead)) {
        setRegisterPin(i, LOW);
      }

    if (i==playhead && playhead != curs && steps[playhead]==0) {
      setRegisterPin(i, HIGH);
    }

    if (i==playhead && playhead != curs && steps[playhead]==1) {
      setRegisterPin(i, LOW);
    }
  }
}

//MENU PAGES

//PLAYMODE PAGE
if (longPressActive) {
    setRegisterPin(0, LOW);
    setRegisterPin(1, LOW);
    setRegisterPin(2, LOW);
    setRegisterPin(3, LOW);

    setRegisterPin(4, LOW);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, LOW);

    setRegisterPin(8, LOW);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, LOW);

    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    setRegisterPin(15, LOW);
  }

//LAST STEP PAGE
if (lastStepActive) {
    setRegisterPin(0, LOW);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, LOW);

    setRegisterPin(4, HIGH);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, HIGH);

    setRegisterPin(8, HIGH);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, HIGH);

    setRegisterPin(12, LOW);
    setRegisterPin(13, HIGH);
    setRegisterPin(14, HIGH);
    setRegisterPin(15, LOW);
  }
  
//RANDOMIZE PAGE
if (randomActive) {
    setRegisterPin(0, HIGH);
    setRegisterPin(1, LOW);
    setRegisterPin(2, LOW);
    setRegisterPin(3, HIGH);

    setRegisterPin(4, LOW);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, LOW);

    setRegisterPin(8, LOW);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, LOW);

    setRegisterPin(12, HIGH);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    setRegisterPin(15, HIGH);
  }

//RESET PAGE
if (resetActive) {
    for (int i=0; i <= 15; i++){
      setRegisterPin(i, HIGH);
    }
  }
  
  //PLAYMODE CURSOR DISPLAY
  if (playModeMenu && !longPressActive && !lastStepMenu && !lastStepActive) {
    for (int i=0; i <= 3; i++){
      if (menuItem[curs]==0) {
          if (currentMillis - previousMillis >= 80) {
            if (blinky==0) {
            setRegisterPin(curs+0, HIGH);
            setRegisterPin(curs+4, HIGH);
            setRegisterPin(curs+8, HIGH);
            setRegisterPin(curs+12, HIGH);
            blinky=1;
            } else {
            setRegisterPin(curs+0, LOW);
            setRegisterPin(curs+4, LOW);
            setRegisterPin(curs+8, LOW);
            setRegisterPin(curs+12, LOW);
            blinky=0;
            }
            previousMillis = currentMillis;
          }
      } else {
          if (currentMillis - previousMillis >= 20) {
            if (blinky==0) {
            setRegisterPin(curs+0, HIGH);
            setRegisterPin(curs+4, HIGH);
            setRegisterPin(curs+8, HIGH);
            setRegisterPin(curs+12, HIGH);
            blinky=1;
            } else {
            setRegisterPin(curs+0, LOW);
            setRegisterPin(curs+4, LOW);
            setRegisterPin(curs+8, LOW);
            setRegisterPin(curs+12, LOW);
            blinky=0;
            }
            previousMillis = currentMillis;
          }
      }
      
      if (menuItem[i]==1 && i != curs) {
        setRegisterPin(i+0, HIGH);
        setRegisterPin(i+4, HIGH);
        setRegisterPin(i+8, HIGH);
        setRegisterPin(i+12, HIGH);
      }
      
      if (menuItem[i]==0 && i != curs) {
        setRegisterPin(i+0, LOW);
        setRegisterPin(i+4, LOW);
        setRegisterPin(i+8, LOW);
        setRegisterPin(i+12, LOW);
      }
      
    }
  }
    //LAST STEP CURSOR DISPLAY
    if (lastStepMenu && !longPressActive && !playModeMenu && !lastStepActive) {
       if (lastStep == curs) {
        if (currentMillis - previousMillis >= 20) {
          if (blinky==0) {
          setRegisterPin(curs, HIGH);
          blinky=1;
          } else {
          setRegisterPin(curs, LOW);
          blinky=0;
          }
          previousMillis = currentMillis;
        }
        } else {
          if (currentMillis - previousMillis >= 80) {
            if (blinky==0) {
            setRegisterPin(lastStep, HIGH);
            blinky=1;
            } else {
            setRegisterPin(lastStep, LOW);
            blinky=0;
            }
          previousMillis = currentMillis;
        }
      }

      for (int i=0; i <= curs; i++){
        if (i!=lastStep){
        setRegisterPin(i, HIGH);
        }
      }
      for (int i=curs+1; i <= 15; i++){
        if (i!=lastStep){
        setRegisterPin(i, LOW);
        }
      }
      
    }
    
  
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//SAVE FUNCTION
void saveState() {
   for (int i=0;i<16;i++){
    saveStepsInput[i].value = steps[i];
   }
        
   for (int i=0;i<4;i++){
    saveStepsInput[i+16].playmode = menuItem[i];
   }
        
    saveStepsInput[20].laststep = lastStep;
        
    // write data to eeprom 
    EEPROM.updateBlock(address, saveStepsInput,21);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void loop() {
  
  currentMillis = millis();
  curs = encoderValue/4;
  updateSteps();
  
  //RESET INPUT
  resIn = digitalRead(6);
  if (resIn != pRes) {
    if (resIn == LOW) {
      //Serial.println("res");
      playhead=-1;
    }
    pRes=resIn;
  }
  
  //TURN OFF THE LED OF THE PREVIOUS CURSOR POSITION
  if (curs != lastcurs && playModeMenu == false) {
    setRegisterPin(lastcurs, LOW); 
    lastcurs = curs;
  }
  
  if (curs != lastcurs && playModeMenu == true) {
    lastcurs = curs;
  }
  
  if (curs != lastcurs && lastStepMenu == true) {
    setRegisterPin(lastcurs, LOW);
    lastcurs = curs;
  }

//BUTTON FUNCTIONS
    if (digitalRead(4) == HIGH) {
      
      //BUTTON ACTIVE
      if (buttonActive == false) {
        buttonActive = true;
        buttonTimer = millis();
      }
      
      //LONG PRESS ACTIVE
      if ((millis() - buttonTimer > playModeTime) && !longPressActive) {
      longPressActive = true;
      }

      //LONG PRESS ACTIVE LAST STEP
      if ((millis() - buttonTimer > lastStepTime) && !lastStepActive && !playModeMenu && !lastStepMenu) {
      lastStepActive = true;
      }

      //LONG PRESS RANDOMIZE
      if ((millis() - buttonTimer > randomTime) && !resetActive && !playModeMenu && !lastStepMenu) {
      randomActive = true;
      }

      //LONG PRESS RESET
      if ((millis() - buttonTimer > resetTime) && !playModeMenu && !lastStepMenu) {
      randomActive=false;
      resetActive = true;
      }
      
    } else {

      //STEP INPUT
      if (buttonActive && !longPressActive && !playModeMenu && !lastStepActive && !lastStepMenu && !resetActive && !randomActive) {
        if (steps[curs]==1) {
          steps[curs]=0;
        } else {
          steps[curs]=1;
        }
        
        saveState();
        saveState();
    
      }


      //ENTER PLAYMODEMENU
      if (longPressActive && !playModeMenu && !lastStepMenu && !lastStepActive && !resetActive && !randomActive) {
        longPressActive = false;
        for (int i=0; i <= 15; i++){
         setRegisterPin(i, LOW);
        }
        
        saveCurs = curs;
        encoderValue = saveMenuCurs*4;
        //encoderValue=0;
        lastStepMenu=false;
        playModeMenu=true;
      }


      //PLAYMODEMENU INPUT
      if (buttonActive && !longPressActive && playModeMenu && !lastStepMenu && !lastStepActive && !resetActive && !randomActive) {
        if (curs==0 && menuItem[0]==false) {menuItem[0]=true;menuItem[1]=false;menuItem[2]=false;menuItem[3]=false;}
        if (curs==1 && menuItem[1]==false) {menuItem[0]=false;menuItem[1]=true;menuItem[2]=false;menuItem[3]=false;}
        if (curs==2 && menuItem[2]==false) {menuItem[0]=false;menuItem[1]=false;menuItem[2]=true;menuItem[3]=false;}
        if (curs==3 && menuItem[3]==false) {menuItem[0]=false;menuItem[1]=false;menuItem[2]=false;menuItem[3]=true;}
      }

      
      //EXIT PLAYMODE SELECT
      if (longPressActive && playModeMenu && !lastStepMenu && !resetActive && !randomActive) {
        longPressActive = false;
        for (int i=0; i <= 15; i++){
         setRegisterPin(i, LOW);
        }
          playModeMenu=false;
          lastStepMenu=false;
          saveMenuCurs = curs;
          encoderValue = saveCurs*4;
          saveState();
          saveState();
      }

      //ENTER LAST STEP
      if (!lastStepMenu && lastStepActive && !playModeMenu && !resetActive && !randomActive) {
        longPressActive = false;
        lastStepActive = false;
        buttonActive=false;
        for (int i=0; i <= 15; i++){
         setRegisterPin(i, LOW);
        }
        
          saveCurs = curs;
          encoderValue = lastStep*4;
          playModeMenu=false;
          lastStepMenu=true;
          //Serial.println("ls ON");
          
      }


      //LASTSTEPMENU INPUT
      if (buttonActive && !longPressActive && !playModeMenu && lastStepMenu && !lastStepActive && !resetActive && !randomActive) {
        lastStep=curs;
        //Serial.println("ls IN");
      }
      

      //EXIT LAST STEP
      if (longPressActive && !playModeMenu && lastStepMenu && !lastStepActive && !resetActive && !randomActive) {
        longPressActive = false;
        lastStepActive = false;
        for (int i=0; i <= 15; i++){
         setRegisterPin(i, LOW);
        }

          playModeMenu=false;
          lastStepMenu=false;
          saveLastStepCurs = curs;
          encoderValue = saveCurs*4;
          saveState();
          saveState();
      }
      //RANDOMIZE
      if (randomActive && !resetActive && !playModeMenu && !lastStepMenu) {
        randomActive=false;
        longPressActive = false;
        lastStepActive = false;
        resetActive=false;
        buttonActive=false;
        playModeMenu=false;
        lastStepMenu=false;
        for (int i=0; i <= 15; i++){
         
         steps[i]=random(2);
        }
        saveState();
        saveState();
      }

      
      //RESET
      if (resetActive && !playModeMenu && !lastStepMenu) {
        randomActive=false;
        longPressActive = false;
        lastStepActive = false;
        resetActive=false;
        buttonActive=false;
        playModeMenu=false;
        lastStepMenu=false;
        for (int i=0; i <= 15; i++){
         steps[i]=0;
        }
        lastStep=15;
        menuItem[0]=true;
        menuItem[1]=false;
        menuItem[2]=false;
        menuItem[3]=false;
        saveState();
        saveState();
      }



      buttonActive = false;
    } 
    
  
  //CLOCK INPUT
  clkin = digitalRead(5);
  if (clkin != pclk) {
    if (clkin == LOW) {
      
      //PLAYMODES:
      
      //FWD MODE
      if (menuItem[0]==1) {
        playhead++;
        if (playhead > lastStep) {
          playhead=0;
        }
      }
      
      //BWD MODE
      if (menuItem[1]==1) {
        playhead--;
        if (playhead < 0) {
          playhead = lastStep;
        }
      }

      //PENDULUM MODE
      if (menuItem[2]==1) {
        if (playhead >= lastStep) {
          changedir=true;
        }
        
        if (playhead <= 0) {
          changedir=false;
        }
        
        if (changedir) {
          playhead--;
        } else {
          playhead++;
        }
      }

      //RAND MODE
      if (menuItem[3]==1) {
        rstep=random(0, lastStep+1);
        playhead=rstep;
      }

      //TURN ON TRIGGER ON OUTPUT
      if (steps[playhead]==1) {
        digitalWrite(11, HIGH);
      } else {
        digitalWrite(11, LOW);
      }
    }
      pMillis = currentMillis;
      pclk = clkin;
  }
  
  //TURN OFF TRIGGER ON OUTPUT
  if (currentMillis - pMillis >= 2) {
            digitalWrite(11, LOW);
  }
  
//LIGHT UP THE APPROPRIATE LEDS
writeRegisters();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//READ ENCODER FUNCTION
void updateEncoder() {
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
      if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
      if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;


    //LIMIT ENCODER VALUE TO APPROPRIATE RANGES
    if (!playModeMenu && !lastStepMenu) {
      if (encoderValue > (lastStep*4+3)) {
      encoderValue = 0;
      }
      
      if (encoderValue < 0) {
      encoderValue = lastStep*4+3;
      }
    }
    
    if (playModeMenu && !lastStepMenu) {
      if ((encoderValue) > 12) {
        encoderValue = 12;
      }
      if (encoderValue < 0) {
        encoderValue = 0;
      }
    }
    
    if (!playModeMenu && lastStepMenu) {
      if ((encoderValue) > 60) {
        encoderValue = 60;
      }
      if (encoderValue < 0) {
        encoderValue = 0;
      }
    }

  lastEncoded = encoded; //store this value for next time
}

