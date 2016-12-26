#include <Arduino.h>
#include <Chrono.h>
Chrono mainTimer(Chrono::SECONDS ); //60min timer
Chrono stepTimer(Chrono::MILLIS ); //20min timer

#define DIST_THRESH 200
//#define master
#define slave

#define NUM_STEPS 6

//BUTTON PINS
#define BUTTON_PIN 10
#define BUTTON_LED_PIN 11

//LEDS
#define STEP_LED_PIN1 2
#define STEP_LED_PIN2 3
#define STEP_LED_PIN3 4
#define STEP_LED_PIN4 5
#define STEP_LED_PIN5 6
#define STEP_LED_PIN6 7


//SENSOR PINS
#define STEP_SENSOR_PIN1 A0
#define STEP_SENSOR_PIN2 A1
#define STEP_SENSOR_PIN3 A2
#define STEP_SENSOR_PIN4 A3
#define STEP_SENSOR_PIN5 A6
#define STEP_SENSOR_PIN6 A7

#define SYNC_INPUT_PIN 9
#define SYNC_OUTPUT_PIN 8
boolean enabled_game = false;
int steps_stage=0;
int step_index=0;
void setup() {


  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SYNC_INPUT_PIN, INPUT);

  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(SYNC_OUTPUT_PIN, OUTPUT);
  pinMode(STEP_LED_PIN1, OUTPUT);
  pinMode(STEP_LED_PIN2, OUTPUT);
  pinMode(STEP_LED_PIN3, OUTPUT);
  pinMode(STEP_LED_PIN4, OUTPUT);
  pinMode(STEP_LED_PIN5, OUTPUT);
  pinMode(STEP_LED_PIN6, OUTPUT);

  digitalWrite(BUTTON_LED_PIN, HIGH);
  digitalWrite(SYNC_OUTPUT_PIN, LOW);

  digitalWrite(STEP_LED_PIN1, LOW);
  digitalWrite(STEP_LED_PIN2, LOW);
  digitalWrite(STEP_LED_PIN3, LOW);
  digitalWrite(STEP_LED_PIN4, LOW);
  digitalWrite(STEP_LED_PIN5, LOW);
  digitalWrite(STEP_LED_PIN6, LOW);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {

  if ( enabled_game == false)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(100);
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        digitalWrite(BUTTON_LED_PIN, LOW);
        digitalWrite(SYNC_OUTPUT_PIN, HIGH);
        enabled_game = true;
        steps_stage = 0;
      }
    }
  }

  if ( enabled_game == true)
  {

    switch (steps_stage) {
      case 0:
        if (digitalRead(SYNC_INPUT_PIN) == HIGH)
        {
          delay(100);
          if (digitalRead(SYNC_INPUT_PIN) == HIGH)
          {
            steps_stage = 1;
            digitalWrite(SYNC_OUTPUT_PIN, LOW);
          }
        }
        break;
      case 1:
        delay(1000);
        digitalWrite(SYNC_OUTPUT_PIN, LOW);
        steps_stage = 2;
        break;

      case 2:
        if (digitalRead(SYNC_INPUT_PIN) == HIGH)
        {
          delay(100);
          if (digitalRead(SYNC_INPUT_PIN) == HIGH)
          {
            step_index++;
            steps_stage = 3;
            if(step_index >5)
            step_index=0;
          }
        }
        break;
      case 3:
        if (readStep[step_index])
        {
          digitalWrite(SYNC_OUTPUT_PIN, HIGH);
          delay(100);
          steps_stage = 4;
        }

      case 4:
        digitalWrite(SYNC_OUTPUT_PIN, LOW);
        steps_stage = 2;
        break;

      case 5: break;
      case 6: break;
      case 7: break;
      case 8: break;

    }

  }



}


bool readStep(int step_num)
{

  switch (step_num) {
    case 0: if (analogRead(STEP_SENSOR_PIN1) < DIST_THRESH) return 1;  break;
    case 1: if (analogRead(STEP_SENSOR_PIN2) < DIST_THRESH) return 1;  break;
    case 2: if (analogRead(STEP_SENSOR_PIN3) < DIST_THRESH) return 1;  break;
    case 3: if (analogRead(STEP_SENSOR_PIN4) < DIST_THRESH) return 1; break;
    case 4: if (analogRead(STEP_SENSOR_PIN5) < DIST_THRESH) return 1; break;
    case 5: if (analogRead(STEP_SENSOR_PIN6) < DIST_THRESH) return 1;  break;
    default: break;

  }

}
