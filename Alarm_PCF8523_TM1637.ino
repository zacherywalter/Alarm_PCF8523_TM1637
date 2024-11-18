#include <Encoder.h>
#include <Arduino.h>
#include <TM1637Display.h>
#include <OneButton.h>
#include <EEPROM.h>
#include "pitches.h"
#include "variables.h"
// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"

// Module connection pins (Digital Pins)
#define CLK 5
#define DIO 6
#define BUTTON_PIN 4
#define MAIN_MENU_TIME 8000 //millis (8seconds)
#define NUM_DISP 6
#define SPEAKER_PIN 8

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int disp_mode = 0;
bool disp_adjust = false;
bool alarm_on = false;
bool sleep_on = false;
long old_position  = -999;
long new_position  = 0;
long diff_position = 0;

long t_start = millis();

uint8_t A1_hour = 8;
uint8_t A1_min = 0;
uint8_t A2_hour = 10;
uint8_t A2_min = 30;
int A1_dec = A1_hour*60 + A1_min;
int A2_dec = A2_hour*60 + A2_min;
bool A1_on = false;
bool A2_on = false;

int time_dec;
int date_dec;

uint8_t time_hour;
uint8_t time_min;

uint8_t date_month;
uint8_t date_day;
int date_year;

long t_start_250mil;
long t_start_500mil;
long t_start_1000mil;
long time_out;

Encoder Enc(2, 3);
TM1637Display display(CLK, DIO);
OneButton button(BUTTON_PIN, true);
RTC_PCF8523 rtc;

DateTime now;

//////////////////////////////////////SETUP////////////////////////////////////////////////
void setup () {
  Serial.begin(57600);
  /*
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 500 / noteDurations[thisNote];
    tone(SPEAKER_PIN, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
  */
  Serial.println("begin");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //Serial.flush();
    while (1) delay(10);
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start();

  // Example - RTC gaining 43 seconds in 1 week
  float drift = 43; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
  float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
  float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
  // float drift_unit = 4.069; //For corrections every min the drift_unit is 4.069 ppm (use with offset mode PCF8523_OneMinute)
  int offset = round(deviation_ppm / drift_unit);
  // rtc.calibrate(PCF8523_TwoHours, offset); // Un-comment to perform calibration once drift (seconds) and observation period (seconds) are correct
  // rtc.calibrate(PCF8523_TwoHours, 0); // Un-comment to cancel previous calibration

  Serial.print("Offset is ");
  Serial.println(offset); // Print to control offset

  button.attachClick(Click, &button);
  button.attachDoubleClick(DoubleClick);
  button.attachLongPressStart(LongPressStart, &button);
  button.attachDuringLongPress(DuringLongPress, &button);
  button.attachLongPressStop(LongPressStop, &button);
  button.setLongPressIntervalMs(1000);
  //button.setDebounceMs(80); //default 50ms

  //recover alarms
  uint8_t A1_hour = EEPROM.read(100);
  uint8_t A1_min = EEPROM.read(101);
  uint8_t A2_hour = EEPROM.read(102);
  uint8_t A2_min = EEPROM.read(103);
}


///////////////////////////////DIPLAY///////////////////////////////////
//void display_output(uint8_t disp_mode, bool disp_adjust, bool disp_blink = false, ){
//  
//}

long time2dec(uint8_t time_hour, uint8_t time_min){
  long time_dec = time_hour*60 + time_min;
  return time_dec;
}
uint8_t dec2hour(long time_dec){
  uint8_t time_hour = floor(float(time_dec)/60);
  return time_hour;
}
uint8_t dec2min(long time_dec){
  uint8_t time_hour = floor(float(time_dec)/60);
  uint8_t time_min = floor(((float(time_dec)/60) - time_hour)*60);
  return time_min;
}

long date2dec(uint8_t date_month, uint8_t date_day){
  long date_dec = (date_month-1)*31 + (date_day-1);
  return date_dec;
}
uint8_t dec2month(long date_dec){
  uint8_t date_month = floor(float(date_dec)/31);
  return date_month+1;
}

uint8_t dec2day(long date_dec){
  uint8_t date_month = floor(float(date_dec)/31);
  uint8_t date_day = floor(((float(date_dec)/31) - date_month)*31);
  return date_day+1;
}

void alarmSound(void){//repetative update required
  if(t_start_1000mil%2 == 0){
    tone(SPEAKER_PIN, NOTE_C7, 50);
    delay(50);
    tone(SPEAKER_PIN, NOTE_E7, 50);
  }
}

/////////////////////////////////LOOP////////////////////////////////////
void loop () {
    now = rtc.now();
    button.tick();
    int k;
    uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
    t_start = millis();
    t_start_250mil = floor(float(t_start)/250);
    t_start_500mil = floor(float(t_start)/500);
    t_start_1000mil = floor(float(t_start)/500);
    
    new_position = Enc.read();
    diff_position = (new_position/4)-(old_position/4);
    if(diff_position>4){
      diff_position*=4;
    }
    Serial.print("diff_pos");
    Serial.println(diff_position);
    if (new_position != old_position) {
      old_position = new_position;
      time_out = millis();
    }
    if(disp_adjust == false){
      disp_mode+=diff_position;
      disp_mode%=NUM_DISP;
      disp_mode = disp_mode < 0 ? NUM_DISP-1 : disp_mode;
      Serial.print("disp_mode");
      Serial.println(disp_mode);
    }

    if(alarm_on){
      alarmSound();
    }

    if(A1_hour == now.hour() && A1_min == now.minute() && !sleep_on && !disp_adjust && A1_on){
      alarm_on = true;
    }
    if(A2_hour == now.hour() && A2_min == now.minute() && !sleep_on && !disp_adjust && A2_on){
      alarm_on = true;
    }
    if(A2_hour != now.hour() || A2_min != now.minute()){
      if(A1_hour != now.hour() || A1_min != now.minute()){
        sleep_on = false;
      }
    }
    if(millis()-time_out>MAIN_MENU_TIME){
      //Serial.println(millis()-time_out);
      disp_mode=0;
    }
    //Mode Control
    if(!disp_adjust){
      if(disp_mode == 0){
          //Main Menu!
          time_dec = time2dec(now.hour(), now.minute());
          date_dec = date2dec(now.month(), now.day());
          date_year = now.year();
          display.setBrightness(0x0f);
          int time_disp = now.hour()*100 + now.minute();
          display.showNumberDecEx(time_disp, (0x40), true);
          //Serial
          Serial.print(now.hour(), DEC);
          Serial.print(':');
          Serial.println(now.minute(), DEC);
      }else if(disp_mode == 1){
          //display the date
          display.setBrightness(0x0f);
          int date_disp = now.day()*100 + now.month();
          display.showNumberDecEx(date_disp, (0x40), true);
          Serial.print(now.day(), DEC);
          Serial.print('/');
          Serial.println(now.month(), DEC);
      }else if(disp_mode == 2){
          //display day of the week
          display.setBrightness(0x0f);
          display.setSegments(DAY[now.dayOfTheWeek()]);
          Serial.println((daysOfTheWeek[now.dayOfTheWeek()]));
      }else if(disp_mode == 3){
          //display the year
          display.setBrightness(0x0f);
          display.showNumberDec(now.year(), true);
          Serial.println(now.year());
      }else if(disp_mode == 4){
          //display Alarm1
          //bool blink_dots = A1_on & (t_start_1000mil%2);
          //Serial.println(blink_dots);
          if(A1_on){
            display.setBrightness(0x0f);
          }else{
            display.setBrightness(0x01);
          }
          int disp_A1 = A1_hour*100 + A1_min;
          display.showNumberDecEx(disp_A1, (0x40), true);
          Serial.print(A1_hour, DEC);
          Serial.print(':');
          Serial.println(A1_min, DEC);
      }else if(disp_mode == 5){
          //display Alarm2
          //bool blink_dots = A2_on & (t_start_1000mil%2);
          //Serial.println(blink_dots);
          if(A2_on){
            display.setBrightness(0x0f);
          }else{
            display.setBrightness(0x01);
          }
          
          int disp_A2 = A2_hour*100 + A2_min;
          display.showNumberDecEx(disp_A2, (0x40), true);
          Serial.print(A2_hour, DEC);
          Serial.print(':');
          Serial.println(A2_min, DEC);
      }
    }else if(disp_adjust == true){
      if(t_start_500mil%2==0){//blink display
        display.setBrightness(0x00);
      }else{
        display.setBrightness(0x0f); 
      }
      if(disp_mode == 0){
        //display time
        time_dec += diff_position;
        if(time_dec >= (24*60)){
          time_dec = 0;
        }
        if(time_dec < 0){
          time_dec = (24*60)-1;
        }
        time_hour = dec2hour(time_dec);
        time_min = dec2min(time_dec);
        int time_disp = time_hour*100 + time_min;
        display.showNumberDecEx(time_disp, (0x40), true);
      }else if(disp_mode == 1){
        //display date
        date_dec += diff_position;
        if(date_dec >= (12*31)){
          date_dec = 0;
        }
        if(date_dec < 0){
          date_dec = (12*31)-1;
        }
        date_month = dec2month(date_dec);
        date_day = dec2day(date_dec);
        int date_disp = date_day*100 + date_month;
        display.showNumberDecEx(date_disp, (0x40), true);
      }else if(disp_mode == 2){
        
      }else if(disp_mode == 3){
        //display year
        date_year += diff_position;
        display.showNumberDec(date_year, true);
      }else if(disp_mode == 4){
        A1_dec += diff_position;
        if(A1_dec >= (24*60)){
          A1_dec = 0;
        }
        if(A1_dec < 0){
          A1_dec = (24*60)-1;
        }
        A1_hour = dec2hour(A1_dec);
        A1_min = dec2min(A1_dec);
        int A1_disp = A1_hour*100 + A1_min;
        display.showNumberDecEx(A1_disp, (0x40), true);
      }else if(disp_mode == 5){
        A2_dec += diff_position;
        if(A2_dec >= (24*60)){
          A2_dec = 0;
        }
        if(A2_dec < 0){
          A2_dec = (24*60)-1;
        }
        A2_hour = dec2hour(A2_dec);
        A2_min = dec2min(A2_dec);
        int A2_disp = A2_hour*100 + A2_min;
        display.showNumberDecEx(A2_disp, (0x40), true);
      }
    }
}

/////////////////////////////////BUTTON_FUNCTIONS/////////////////////////////////////
void DoubleClick(){
  //Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("DoubleClick");
  alarm_on = false;
  sleep_on = true;
  if (disp_adjust == false){
    if(disp_mode == 4){
      A1_on = !A1_on;
    }else if(disp_mode == 5){
      A2_on = !A2_on;
    }
  }
}

void Click(void *oneButton){
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - click()");
  time_out = millis();
  if(disp_adjust == true){
    tone(SPEAKER_PIN, NOTE_G4, 50);//succesful adjust tone
    delay(50);
    tone(SPEAKER_PIN, NOTE_C5, 50);
    disp_adjust = false;
    if(disp_mode == 0){
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), time_hour, time_min, 0));
    }else if(disp_mode == 1){
      rtc.adjust(DateTime(now.year(), date_month, date_day, now.hour(), now.minute(), 0));
      while(!now.isValid()){
        Serial.println("Invalid date, removing a day");
        Serial.println(date_day);
        tone(SPEAKER_PIN, NOTE_C1, 10);
        date_day--;
        rtc.adjust(DateTime(now.year(), date_month, date_day, now.hour(), now.minute(), 0));
      }
    }else if(disp_mode == 2){
      
    }else if(disp_mode == 3){
      rtc.adjust(DateTime(date_year, now.month(), now.day(), now.hour(), now.minute(), 0));
    }else if(disp_mode == 4){
      EEPROM.write(100, A1_hour);
      EEPROM.write(101, A1_min);
      A1_on = true;
    }else if(disp_mode == 5){
      EEPROM.write(102, A2_hour);
      EEPROM.write(103, A2_min);
      A2_on = true;
    }
  }
}

void LongPressStart(void *oneButton){
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStart()");
  time_out=millis();
  if(disp_adjust == false){
    tone(SPEAKER_PIN, NOTE_C6, 50);//adjust tone
    disp_adjust = true;
  }
}

void LongPressStop(void *oneButton){
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStop()\n");
}

void DuringLongPress(void *oneButton){
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - DuringLongPress()");
}
