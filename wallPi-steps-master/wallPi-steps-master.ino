/*
  master
*/

#include <Arduino.h>
#include <Chrono.h>

char startGameB[] = "STARTB";
char stopGameB[] = "STOPB:%d";

Chrono gameTimer(Chrono::SECONDS ); //2min timer
Chrono stepTimer(Chrono::MILLIS ); //1000 ms timer

#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define NUM_STEPS 6
#define WRITE_EN_PIN 12
#define DIST_THRESH 200

#define MAX_STEPS_GAME 10
#define MIN_STEPS_GAME 3

//BUTTON PINS
#define BUTTON_PIN 10
#define BUTTON_LED_PIN 11

int syncINstate = 0;         // current state of the sync in
int lastsyncINstate = 0;     // previous state of the sync in

//LEDS
#define STEP_LED_PIN1 5
#define STEP_LED_PIN2 2
#define STEP_LED_PIN3 3
#define STEP_LED_PIN4 7
#define STEP_LED_PIN5 4
#define STEP_LED_PIN6 6

//SENSOR PINS
#define STEP_SENSOR_PIN1 A5
#define STEP_SENSOR_PIN2 A3
#define STEP_SENSOR_PIN3 A4
#define STEP_SENSOR_PIN4 A2
#define STEP_SENSOR_PIN5 A1
#define STEP_SENSOR_PIN6 A0

#define SYNC_INPUT_PIN 9
#define SYNC_OUTPUT_PIN 8

boolean enabled_game = false;
int steps_stage = 0;
int step_index = 0;
unsigned int correctSteps = 0;
unsigned int stepAttempts = 0;
int result = 0;
unsigned int resultSlave = 0;
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
    digitalWrite(BUTTON_LED_PIN, HIGH);
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(100);
      if (digitalRead(BUTTON_PIN) == LOW)
      {

         if (digitalRead(SYNC_INPUT_PIN) == HIGH)
        {
           delay(100);
           if (digitalRead(SYNC_INPUT_PIN) == HIGH)
           {
            digitalWrite(BUTTON_LED_PIN, LOW);
            digitalWrite(SYNC_OUTPUT_PIN, HIGH);
            enabled_game = true;
            steps_stage = 0;
           }
        }     
      }
    }
  }else{
    digitalWrite(BUTTON_LED_PIN, LOW);
  }

  if (enabled_game == true)
  {
    if (stepsGame())
    {
      char command[100] = "";
      sprintf(command,
              stopGameB,
              result
             );
      digitalWrite(WRITE_EN_PIN, RS485Transmit);
      delay(100);
      Serial.print(command);
      Serial.print("\n");
      delay(100);
      digitalWrite(WRITE_EN_PIN, RS485Receive);

      enabled_game = false;
      endBlink();
       digitalWrite(BUTTON_LED_PIN, HIGH);
    }
  }



}//end loop


void endBlink() {
  for (int i = 0; i < 5 ; i++)
  {
    digitalWrite(BUTTON_LED_PIN, HIGH);
    delay(100);
    digitalWrite(BUTTON_LED_PIN, LOW);
    delay(100);
  }
  digitalWrite(BUTTON_LED_PIN, HIGH);
}

int stepsGame() {

  switch (steps_stage) {

    case 0:
      step_index = 0;
      correctSteps = 0;
      stepAttempts = 0;
      resultSlave=0;
      for (int i = 0; i < NUM_STEPS ; i++)
      {
        offStepLed(i);
      }
      steps_stage = 1;
      digitalWrite(SYNC_OUTPUT_PIN, HIGH);
      break;

    case 1:

        gameTimer.restart();
        stepTimer.restart();

        digitalWrite(WRITE_EN_PIN, RS485Transmit);
        delay(100);
        Serial.print(startGameB);
        Serial.print("\n");
        delay(100);
        digitalWrite(WRITE_EN_PIN, RS485Receive);

        steps_stage = 2;
        digitalWrite(SYNC_OUTPUT_PIN, LOW); 

      break;
    case 2:
    

      steps_stage = 3;
       onStepLed(step_index);
      break;

    case 3:

      syncINstate = digitalRead(SYNC_INPUT_PIN);

      if (syncINstate != lastsyncINstate)
      {

        if (digitalRead(SYNC_INPUT_PIN) == HIGH)
        {
          resultSlave++;
        }

      }

      lastsyncINstate = syncINstate;

      if (gameTimer.hasPassed(120))
      {
        steps_stage = 4;
        break;
      }

      if (stepTimer.hasPassed(1000)) //change target on 1,4s?
      {
        offStepLed(step_index);

        step_index++;
        onStepLed(step_index);

        stepAttempts++;
        if (step_index > NUM_STEPS)
          step_index = 0;

        steps_stage = 2;
        stepTimer.restart();
      }
      else

        if (readStep(step_index))
        {
       
            //Serial.print(step_index); Serial.println("STEP");
            stepBlink(step_index);
            offStepLed(step_index);
            correctSteps++;
            delay(1000);
            //targetTimer.restart();
            //Serial.println("hit");
          step_index++;
          
        }
      break;
    case 4:
      digitalWrite(SYNC_OUTPUT_PIN, HIGH);
      steps_stage = 0;
//     Serial.print("slave result: ");  Serial.println(resultSlave);
//     Serial.print("master result: "); Serial.println(result);
//     Serial.print("correctSteps: ");  Serial.println(correctSteps);
//     Serial.print("stepAttempts: "); Serial.println(stepAttempts);
//      
      if (correctSteps <= stepAttempts)
      {
        result = map(correctSteps, 0, stepAttempts, MIN_STEPS_GAME+1 , MAX_STEPS_GAME);
      } else
        result = MAX_STEPS_GAME - 1;

      for (int i = 0; i < NUM_STEPS; i++)
      {
        offStepLed(i);
      }
      digitalWrite(SYNC_OUTPUT_PIN, LOW);
      return result;
      break;
  }
  return 0;

}


bool readStep(int step_num)
{
  switch (step_num)
  {
    case 0: if (analogRead(STEP_SENSOR_PIN1) < DIST_THRESH) return 1;  break;
    case 1: if (analogRead(STEP_SENSOR_PIN2) < DIST_THRESH) return 1;  break;
    case 2: if (analogRead(STEP_SENSOR_PIN3) < DIST_THRESH) return 1;  break;
    case 3: if (analogRead(STEP_SENSOR_PIN4) < DIST_THRESH) return 1; break;
    case 4: if (analogRead(STEP_SENSOR_PIN5) < DIST_THRESH) return 1; break;
    case 5: if (analogRead(STEP_SENSOR_PIN6) < DIST_THRESH) return 1;  break;
    default: break;
  }
  return 0;
}

void onStepLed(int step_num)
{
  switch (step_num)
  {
    case 0:  digitalWrite(STEP_LED_PIN1, HIGH); break;
    case 1:  digitalWrite(STEP_LED_PIN2, HIGH); break;
    case 2:  digitalWrite(STEP_LED_PIN3, HIGH); break;
    case 3:  digitalWrite(STEP_LED_PIN4, HIGH); break;
    case 4:  digitalWrite(STEP_LED_PIN5, HIGH); break;
    case 5:  digitalWrite(STEP_LED_PIN6, HIGH); break;
    default: break;
  }
}

void offStepLed(int step_num)
{
  switch (step_num)
  {
    case 0:  digitalWrite(STEP_LED_PIN1, LOW); break;
    case 1:  digitalWrite(STEP_LED_PIN2, LOW); break;
    case 2:  digitalWrite(STEP_LED_PIN3, LOW); break;
    case 3:  digitalWrite(STEP_LED_PIN4, LOW); break;
    case 4:  digitalWrite(STEP_LED_PIN5, LOW); break;
    case 5:  digitalWrite(STEP_LED_PIN6, LOW); break;
    default: break;
  }
}

void stepBlink(int num ) {
  for (int i = 0; i < 2; i++)
  {
    onStepLed(num);
    delay(100);
    //Serial.println("hit");
    offStepLed(num);
    delay(50);
  }
}
