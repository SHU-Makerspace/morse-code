/**
 * IDEA LAB (Makerspace)
 * Sacred Heart University
 * 
 * Morse code receiver
 * Wiring is 
 * Author: Cedric Bleimling
 * Licence: CC NC SA
 */
 
#include <LiquidCrystal_I2C.h>

const char MorseTree[] = {'\0','E', 'T', 'I', 'A', 'N', 'M', 'S',
                          'U', 'R', 'W', 'D', 'K', 'D', 'O', 'H',
                          'V', 'F', 'U', 'L', 'A', 'P', 'J', 'B',
                          'X', 'C', 'Y', 'Z', 'Q', '\0','\0','5',
                          '4', '\0','3', '\0','\0','\0','2', '\0',
                          '\0','+', '\0','\0','\0','\0','1', '6',
                          '=', '/', '\0','\0','\0','(', '\0','7',
                          '\0','\0','\0','8', '\0','9', '0', '\0',
                          '\0','\0','\0','\0','\0','\0','\0','\0',
                          '\0','\0','\0','?', '_', '\0','\0','\0',
                          '\0','"', '\0','\0','.', '\0','\0','\0',
                          '\0','@', '\0','\0','\0','\0','\0','\0',
                          '-', '\0','\0','\0','\0','\0','\0','\0',
                          '\0',';', '!', '\0',')', '\0','\0','\0',
                          '\0','\0',',', '\0','\0','\0','\0',':',
                          '\0','\0','\0','\0','\0','\0','\0'
                         };




// LCD inititilisation
LiquidCrystal_I2C lcd(0x3F,16,2);

int val = 0; // Light value fron the LDR
int lightHigh = 0;
int lightLow = 0;
int codePtr = 0;
int timeUnitLen;
int previousLen = 200;
bool notAnalysed = false; // Keep tracks of wether or not we have treated the previous "State"
bool endOfTrans = true; // Are we still transmitting? Important to print the last letter of the transmission
static unsigned long timer = millis();
bool lightState = false; // state of the light
int lightOnLen = 0;
int lightOffLen = 0;
int threshold = 0; //Will hold the threshold of the photoresistor to differanciate a dash from a dot.



//--------------------
// STUDENT SETTINGS
//--------------------
bool debug = false; // General debug
bool debugSensor = true; // debug the light value threshold
bool debugtimeUnitLen = true; // timeUnitLen debug -> Only displays when touching the potentiometer -> can leave always on
bool debugTiming = false; // debug the morse unit rules (Some clients don't follow the normal rules and their spaces and end of word are too short!)
bool debugAdv = false; // Advanced debug: prints all the decision values
//--------------------

//--------------------
// STUDENT SETTINGS
//--------------------
bool debug = false; // General debug
bool debugSensor = true; // debug the light value threshold
bool debugtimeUnitLen = true; // timeUnitLen debug -> Only displays when touching the potentiometer -> can leave always on
bool debugTiming = false; // debug the morse unit rules (Some clients don't follow the normal rules and their spaces and end of word are too short!)
bool debugAdv = false; // Advanced debug: prints all the decision values
//--------------------


////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Read below at your own risk */

void getMorse(){    

    // Setting the time unit length from the potentiometer
    previousLen = timeUnitLen;
    timeUnitLen = map(analogRead(A1),0,1023,0,500);
    timeUnitLen = (timeUnitLen/5 + (timeUnitLen%5>2)) * 5; // Round to closest 5
    if(debugtimeUnitLen){ 
        if(abs(timeUnitLen-previousLen)>2) {
          Serial.println("timeUnitLen:" + String(timeUnitLen));
          // Display of the value on the screen for ease of use.
          lcd.clear();
          lcd.print("Time Unit Length");
          lcd.setCursor(0, 1);
          lcd.print(timeUnitLen);
          delay(500);
          lcd.clear();          
        } 
    }
    // Adjusting the time unit to 95% of it's value to avoid misreading if the sender and receiver are slighlty out of sync (Effet de bord)
    //timeUnitLen = timeUnitLen * 0.95;


    // Reading the LDR light value
    val = analogRead(A0);
    if(debugSensor){ Serial.println("Value is :" + String(val) + " - Threshold: " + threshold); }

    ////////////////////
  if (val >= threshold)
  {
    // first "high" of the light
    if(!lightState){
     lightOffLen = millis()-timer;
     timer=millis();
     lightState = true;     
     notAnalysed = true;
     endOfTrans = false;
     if(debugTiming){ Serial.println("Lenght Off: " + String(lightOffLen) + " or " + String(lightOffLen/timeUnitLen) + " units"); }
     lightOffLen = lightOffLen/timeUnitLen; // We count as units of times defined by timeUnitLen
    }
    if(debug){ digitalWrite(13, HIGH); }
  } else {
    // first "low" of the light
    if(lightState){
     lightOnLen = millis()-timer;
     timer=millis();
     lightState = false;
     notAnalysed = true;
     endOfTrans = false;
     if(debugTiming){ Serial.println("Lenght On: " + String(lightOnLen) + " or " + String(lightOnLen/timeUnitLen) + " units"); }
     lightOnLen = lightOnLen/timeUnitLen; // We count as units of times defined by timeUnitLen
    }
    if(debug){ digitalWrite(13, LOW); }

    // End of Transmission + finish up the last letter
    if(!endOfTrans && ((millis()-timer)/timeUnitLen) >= 10){
          if(debug){ Serial.println("---------------- Letter found: " + String(MorseTree[codePtr]) + " ---------------------"); }
          Serial.print(MorseTree[codePtr]);
          lcd.print(MorseTree[codePtr]);
          codePtr = 0;
          endOfTrans = true;
          if(debug){ Serial.println(" End of Transmission"); }
          Serial.println("");
          Serial.println("---------------------------------------------------------------");
    }

  }

    /*** Morse decoding ***/

    // When light is off, we can check if short or long signal was given
    if(notAnalysed && !lightState){
      if ( lightOnLen <= 1) {
        Serial.print(".");
        codePtr = (2*codePtr) + 1;
        notAnalysed = false;
      } else if (lightOnLen >= 2) {
        Serial.print("-");
        codePtr = (2*codePtr) + 2;
        notAnalysed = false;
      } else if (lightOnLen >= 3) {
        Serial.println("********* Your stay on too long ************");
      }
    }

    // When light is on we check for spaces and ends of words or transmission
    if(notAnalysed && lightState){
        if(lightOffLen <= 1){
          // This is a "regular" off between on for now, we do nothing
          notAnalysed = false;
        } else if(lightOffLen >= 2 && lightOffLen < 5){
          if(debug){ Serial.println("---------------- Letter found: " + String(MorseTree[codePtr]) + " ---------------------"); }
          Serial.print(MorseTree[codePtr]);
          lcd.print(MorseTree[codePtr]);
          codePtr = 0;
          notAnalysed = false;
        } else if(lightOffLen >= 5 && lightOffLen < 10){
          if(debug){ Serial.println("---------------- Letter found: " + String(MorseTree[codePtr]) + " ---------------------"); }
          Serial.print(MorseTree[codePtr]);
          lcd.print(MorseTree[codePtr]);
          codePtr = 0;
          notAnalysed = false;
          if(debug){ Serial.println(" End of Word "); }
          Serial.print("#");
          lcd.print("#");
        } else if(lightOffLen > 50){
          // Light was off for so long, this is a new transmission
          lcd.clear();
        } 
    }

    if(debugAdv){
        Serial.println("lightOnLen: " + String(lightOnLen) + " - lightOffLen: " + String(lightOffLen) + " - timeUnitLen: " + String(timeUnitLen) + " - Bflag: " + String("") + " - codePtr: " + String(codePtr));
      }

    if(debug){ digitalWrite(13, LOW); }
}


// threshold value for the photoresistor to differenciate between (light on) and (light off)
void calibrateThreshold()
{
	int initValue = map(analogRead(A1),0,1023,0,500);
	threshold = initValue*1.25; // Threshoold is 25% more than the current reading of light.
}


void setup()
{ 
  pinMode(13,OUTPUT); // debug led
  pinMode(A1,INPUT); // photoresistor
  Serial.begin(115200);// Start a Serial Connection
  Serial.println("-----------------------");
  Serial.println("Sacred Heart University");
  Serial.println("------ IDEA Lab -------");
  Serial.println("-----------------------");
  Serial.println("- Morse Code Receiver -");
  Serial.println("-----------------------");
  Serial.println("");
  delay(50);
   
    // set up the LCD's number of columns and rows:
  Wire.begin();
  lcd.begin(16,2);
  lcd.backlight();//To Power ON the back light
  // Print a message to the LCD.
  lcd.setCursor(0,0);
  lcd.print("SHU MAKERSPACE");
  lcd.setCursor(0, 1);
  lcd.print("Morse Receiver");
  delay(2000);
  lcd.clear();
  
  // Auto calibrate the photoresistor to ambiant lighting:
  calibrateThreshold();
  
}

void loop()
{
  getMorse();
}
