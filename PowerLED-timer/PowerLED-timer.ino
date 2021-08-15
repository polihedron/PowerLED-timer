#include <TM1637Display.h>                                            // https://github.com/avishorp/TM1637
#include <ClickEncoder.h>                                             // https://github.com/0xPIT/encoder/tree/arduino
#include <TimerOne.h>                                                 // https://playground.arduino.cc/Code/Timer1/
#include <EEPROM.h>

#define numberOfcentiSeconds( _time_ ) (( _time_ / 10 ) % 100 )       // amount of centiseconds
#define numberOfSeconds( _time_ ) (( _time_ / 1000 ) % 60 )           // amount of seconds
#define numberOfMinutes( _time_ ) ((( _time_ / 1000 ) / 60 ) % 60 )   // amount of minutes 

#define config_version "v1"
#define config_start 16

//#define LowLED
#define HighLED

//#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x) 
#endif


const int enc_pin_A = 5;                                              // rotary encoder first data pin A at D5 pin
const int enc_pin_B = 4;                                              // rotary encoder second data pin B at D4 pin, if the encoder moved in the wrong direction, swap A and B
const int enc_pin_SW = 7;                                             // rotary encoder switch button at D7 pin
const int en_pin = 6;                                                 // LED mosfet enable at D6 pin  

#ifdef LowLED
const int pwr_pin = 8;                                                // ledx12 power switch using relay at D8 pin 
#endif

#ifdef HighLED
const int pwr_pin = 10;                                               // ledx18 power switch using relay at D10 pin 
#endif

bool pwrset = false;                                                  // no power menu at start
bool colon = true;                                                    // timer colon active at start
bool done = true;                                                     
int PWR, lastPWR, timerMinutes, timerSeconds;
int16_t value, lastValue;
unsigned long colon_ms, timeLimit, timeRemaining, savemillis, himillis;

uint8_t save[] = {
  SEG_A|SEG_F|SEG_G|SEG_C|SEG_D,                                      // S
  SEG_A|SEG_B|SEG_F|SEG_G|SEG_E|SEG_C,                                // A
  SEG_F|SEG_E|SEG_C|SEG_D|SEG_B,                                      // V
  SEG_A|SEG_F|SEG_D|SEG_G|SEG_E,                                      // E
};

uint8_t hi[] = {
  0x00, 0x00,
  SEG_F|SEG_B|SEG_G|SEG_C|SEG_E,                                      // H
  SEG_B|SEG_C,                                                        // I
};


TM1637Display display( 2, 3 );                                        // TM1637 CLK connected to D2 and DIO to D3

ClickEncoder *encoder;



void timerIsr() {                                                     // encoder interupt service routine
  
  encoder -> service();  
  
}

typedef struct {                                                      // settings to save in eeprom
    char version[3];
    int timerMinutes;
    int PWR;
    
} settings;


settings cfg = {
     config_version,
     15,                                                              // int timerMinutes
     50                                                               // int PWR
};



bool loadConfig() {                                                   

  if (EEPROM.read( config_start + 0 ) == config_version[0] &&
      EEPROM.read( config_start + 1 ) == config_version[1] ){

    for (int i = 0; i < sizeof( cfg ); i++ ){
      *(( char* )&cfg + i) = EEPROM.read( config_start + i );
    }
    DEBUG_PRINTLN( "configuration loaded:" );
    DEBUG_PRINTLN( cfg.version );

    timerMinutes = cfg.timerMinutes;
    PWR = cfg.PWR;
    return true;

  }
  return false;

}



void saveConfig() {
  for ( int i = 0; i < sizeof( cfg ); i++ )
    EEPROM.write( config_start + i, *(( char* )&cfg + i ));
    DEBUG_PRINTLN( "configuration saved" );
}



void setup() {
  
  Serial.begin( 115200 );                                             // serial for debug

  display.setBrightness( 0x04 );                                      // min brightness, to reduce LM1117-5V current and overheating, maximum is 7
 
  encoder = new ClickEncoder( enc_pin_A, enc_pin_B, enc_pin_SW, 4 );  // rotary encoder init, last value is encoder steps per notch
  
  Timer1.initialize( 1000 );                                          // set the timer period in us, 1ms
  Timer1.attachInterrupt( timerIsr );                                 // attach the service routine
  
  pinMode( en_pin, OUTPUT );   
  pinMode( pwr_pin, OUTPUT );        
  digitalWrite(en_pin, LOW);   
  digitalWrite(pwr_pin, LOW);                             
  lastValue = 0;
  PWR = 50;                                                           // 50% LED power as default
  colon_ms = millis();
  savemillis = -2000;
  himillis = 0;
  if ( !loadConfig() ) {                                              // checking and loading configuration
    DEBUG_PRINTLN( "configuration not loaded!" );
    saveConfig();                                                     // default values if no config
  }

}



void menuTimer() {
  
   unsigned long runTime;
   timeLimit = 0;
  
   while  ( done && !pwrset )  {
      value += encoder -> getValue();
            if ( value > lastValue ) {
              timerMinutes++;                                         // one rotary step is 1 minute
              if ( timerMinutes >= 60 ) {
                timerMinutes = 60; 
                 timerSeconds = 0;                                    // max 60 minutes
              }  
            } 
            else if ( value < lastValue ) {
             if (timerMinutes > 0 ) {
                timerMinutes--;
                 timerSeconds = 0;

              } 
            }
            if ( value != lastValue ) {
              lastValue = value;
              DEBUG_PRINT( "Encoder value: " );
              DEBUG_PRINTLN( value );
            }
            
    if ( millis() - himillis < 2000 )                                 // say HI at power on                          
      display.setSegments( hi, 4, 0 );  
    else if ( millis() - savemillis < 2000 )                          // show SAVE if saving config to eeprom                          
      display.setSegments( save, 4, 0 );
    else                                                              // display time to countdown, leading zeros active if no hours, colon active
      display.showNumberDecEx( timeToInteger( timerMinutes, timerSeconds ), 0x80 >> true , timerMinutes == 0 );
    
    buttonCheck();                                                    // check if rotary encoder button pressed
    
  }
   
  runTime = millis();                                                 // 1000 ms = 1s, so 1 minute is 60000 ms, and 1 hour is 3600000 ms
  timeLimit = timerMinutes * 60000 + timerSeconds * 1000 + runTime;   // add the runtime until timer starts to timeLimit, limit is compared with mcu millis in main loop
   
}



void menuPWR()  {
  
  value += encoder -> getValue();
          if ( value > lastValue ) {                                   
              PWR = 100;                                               // max LED power 100%
                                                                     
            } 
          if ( value < lastValue )
{
              PWR = 50;                                                // min LED power 50%
}
            
    if ( lastPWR != PWR ) {
      DEBUG_PRINT( "PWR value: " );
      DEBUG_PRINTLN( PWR );
        if  (PWR == 50) {                                              // 50% is for 350mA current, 100% is for 700mA 
          digitalWrite( pwr_pin, LOW );
        }
        else if  (PWR == 100)   {
          digitalWrite( pwr_pin, HIGH );
        }
      }
      
     
      lastPWR = PWR;                                                 
      
  
  if ( value != lastValue ) lastValue = value;
  
  display.showNumberDecEx( PWR, 0x80 >> false , false );              // show power value, no colon, no leading zeros
  
  buttonCheck();                                                      // check rotary encoder button
  timeCheck();                                                        // check timer if finished
  
}



void countdown() {

  int n_centisec = numberOfcentiSeconds( timeRemaining );             // amount of centiseconds in remaining time
  int n_seconds = numberOfSeconds( timeRemaining );                   // amount of seconds in remaining time
  int n_minutes = numberOfMinutes( timeRemaining );                   // amount of minutes in remaining time 

  
   if (( millis() - colon_ms ) >= 500 ) {                             // colon is blinking with about 0.5s period
        colon_ms = millis();
        colon =! colon;
        if ( colon ) {                                                // print timer countdown with about 1s period

          if ( n_minutes )  {
           DEBUG_PRINT( n_minutes );
            DEBUG_PRINT( " Minutes " );
          }  
          if ( n_seconds )  {
          DEBUG_PRINT( n_seconds );
          DEBUG_PRINTLN( " Seconds" );
          }

        }
        
   }
                                                
   if ( n_minutes <= 0 ) {                                                  
                                                                      // show seconds and centiseconds if no minutes left
       n_minutes = n_seconds;
        n_seconds = n_centisec;

   }
   
                                                                      // show time, minutes in first two positions, with colon and leading zeros enabled 
                                                                      
   display.showNumberDecEx( timeToInteger( n_minutes, n_seconds ), 0x80 >> colon, n_minutes == 0 );

   buttonCheck();                                                     // check rotary encoder button
   timeCheck();                                                       // check timer if finished
   
}



int timeToInteger( int _minutes, int _seconds ) {
  
  int result = 0;
  result += _minutes * 100;
  result += _seconds;
  
  return result;
  
}


void buttonCheck() {
  
 ClickEncoder::Button b = encoder -> getButton();
   if ( b != ClickEncoder::Open ) {
      DEBUG_PRINT( "Button: " );
      
      #define VERBOSECASE( label ) case label: Serial.println( #label ); break;
      
      switch ( b ) {
         VERBOSECASE( ClickEncoder::Pressed );
         VERBOSECASE( ClickEncoder::Released )
         
       case ClickEncoder::Clicked:
         DEBUG_PRINTLN( "ClickEncoder::Clicked" );
         if ( !isTimerFinished() )  {                                 // can't set power or start countdown if timer not set (00:00)
            if ( !pwrset )  {                                         // set led power, 50% is for 350mA current, 100% is for 700mA 

                digitalWrite(en_pin, HIGH);
                
                if  (PWR == 50) {
                  digitalWrite( pwr_pin, LOW );
                }
                else if  (PWR == 100) {
                  digitalWrite( pwr_pin, HIGH );
                }
                pwrset = true;
                DEBUG_PRINTLN( "PWR set" );
            }
            else {                                                    // start or go back to countdown if pwr set 
              done = false;
              pwrset = false;
              DEBUG_PRINTLN( "Countdown" );
            } 
         }
       break;
                
       case ClickEncoder::Held:                                       // timer reset if rotary encoder button held for about 2s
         DEBUG_PRINTLN( "ClickEncoder::Held" );
         if ( !done ) timerFinished();
       break;

        case ClickEncoder::DoubleClicked:                             // save config if rotary encoder button double clicked
         DEBUG_PRINTLN( "ClickEncoder::DoubleClicked" );
          if ( done && !isTimerFinished() ) {
            cfg.timerMinutes = timerMinutes;
            cfg.PWR = PWR;
            saveConfig();
            savemillis = millis();                                    // time marker used to show SAVE on display - menuTimer()
          }
       break;

       
      } 
   }
}



void timeCheck() {
  
  timeRemaining = timeLimit - millis();                               // calculate time remaining
    
  if ( timeRemaining < 500 ) timerFinished();                         // timer reset if coundown finished

}



bool isTimerFinished() {
  
  return timerMinutes == 0 && timerSeconds == 0;  
  
}



void timerFinished()  {
                                                                      // timer reset, turn off LED
  timerMinutes = 0;
  timerSeconds = 0; 
  value = encoder -> getValue();
  lastValue = value;                                                  // set last encoder value
  pwrset = false;
  done = true;
  digitalWrite( en_pin, LOW );
  digitalWrite( pwr_pin, LOW );
  DEBUG_PRINTLN( "Timer finished" );
  
}



void loop() {
  
  if ( !pwrset ) {
    if ( done ) menuTimer();
    else countdown();
  }
  else
    menuPWR();

}
