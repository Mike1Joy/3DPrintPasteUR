//Calibrate steppers
//Refine power supply wiring + box
//Work out Cura script

#include <math.h>
#include <Tone.h>


////// PIN LAYOUT //////

#define EN_PIN_0 5
#define DIR_PIN_0 7
#define PUL_PIN_0 6

#define EN_PIN_1 13
#define DIR_PIN_1 12
#define PUL_PIN_1 11

#define D0 38
#define D1 40
#define D2 42
#define D3 44

#define D4_MOTOR0 46
#define D5_MOTOR1 48
#define D6_WOUTPROG 50
#define D7_PROGRUN 52

#define A_GND A0
#define A_0 A7
#define A_1 A15


////// CONSTANTS //////

#define FORWARD_0 HIGH      // switch HIGH and LOW to change forward direction of motor
#define BACKWARD_0 LOW
#define PUL_PER_REV_0 3200
#define RETRACT_RPM_0 23.2f
#define MAXRPM_0 23.2f      // RPM of motor 0 when at max speed (b_speed = 14)

#define FORWARD_1 HIGH
#define BACKWARD_1 LOW
#define PUL_PER_REV_1 3200
#define RETRACT_RPM_1 170
#define MAXRPM_1 170        // RPM of motor 1 when at max speed (b_speed = 14)

#define VOLTAGE_1 231       // value from analog read when robot set to 1v
#define VOLTAGE_4 825       // value from analog read when robot set to 4v


////// VARIABLES //////

int b0;
int b1;
int b2;
int b3;

bool Motor0;
bool Motor1;
bool RunWithoutProg;
bool ProgRunning;

int a0;
int a1;

int b_speed;

float a0_speed;
float a1_speed;

float Hz_mult_0;
int Retract_Hz_0;

float Hz_mult_1;
int Retract_Hz_1;

Tone stepper0;
Tone stepper1;


////// PROGRAM //////

void setup()
{
  // OUT
  pinMode(EN_PIN_0, OUTPUT);
  digitalWrite(EN_PIN_0, HIGH); //(LOW active)
  pinMode(DIR_PIN_0, OUTPUT);
  digitalWrite(DIR_PIN_0, FORWARD_0);
  pinMode(PUL_PIN_0, OUTPUT);
  digitalWrite(PUL_PIN_0, LOW);

  pinMode(EN_PIN_1, OUTPUT);
  digitalWrite(EN_PIN_1, HIGH); //(LOW active)
  pinMode(DIR_PIN_1, OUTPUT);
  digitalWrite(DIR_PIN_1, FORWARD_0);
  pinMode(PUL_PIN_1, OUTPUT);
  digitalWrite(PUL_PIN_1, LOW);

  stepper0.begin(PUL_PIN_0);
  stepper1.begin(PUL_PIN_1);

  // IN
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);

  pinMode(D4_MOTOR0, INPUT);
  pinMode(D5_MOTOR1, INPUT);
  pinMode(D6_WOUTPROG, INPUT);
  pinMode(D7_PROGRUN, INPUT);

  pinMode(A_0,INPUT);
  pinMode(A_1,INPUT);

  // INIT
  b0 = 0;
  b1 = 0;
  b2 = 0;
  b3 = 0;

  Motor0 = 0;
  Motor1 = 0;
  RunWithoutProg = 0;
  ProgRunning = 0;

  a0 = 0;
  a1 = 0;

  b_speed = 0;

  a0_speed = 1.0f;
  a1_speed = 1.0f;

  Hz_mult_0    = ((float)MAXRPM_0     )*((float)PUL_PER_REV_0)/(60.0f*14);
  Retract_Hz_0 = ((float)RETRACT_RPM_0)*((float)PUL_PER_REV_0)/(60.0f   );

  Hz_mult_1    = ((float)MAXRPM_1     )*((float)PUL_PER_REV_1)/(60.0f*14);
  Retract_Hz_1 = ((float)RETRACT_RPM_1)*((float)PUL_PER_REV_1)/(60.0f   );

  Serial.begin(9600);
}

void GetInputs()
{
  b0 = 1*!digitalRead(D0);
  b1 = 2*!digitalRead(D1);
  b2 = 4*!digitalRead(D2);
  b3 = 8*!digitalRead(D3);

  b_speed = b0 + b1 + b2 + b3;

  Motor0 = !digitalRead(D4_MOTOR0);
  Motor1 = !digitalRead(D5_MOTOR1);
  RunWithoutProg = !digitalRead(D6_WOUTPROG);
  ProgRunning = !digitalRead(D7_PROGRUN);

  analogRead(A_GND); // discharge ADC capacitor
  a0 = analogRead(A_0);
  analogRead(A_GND); // discharge ADC capacitor
  a1 = analogRead(A_1);
}

void ProcessAnalog()
{
  a0_speed = constrain(map(a0, VOLTAGE_1, VOLTAGE_4, 10, 40), 0, 50)*0.1f;
  a1_speed = constrain(map(a1, VOLTAGE_1, VOLTAGE_4, 10, 40), 0, 50)*0.1f;
}

void PrintInputs()
{
  Serial.print("b0: ");
  Serial.print(b0);
  Serial.print("  b1: ");
  Serial.print(b1);
  Serial.print("  b2: ");
  Serial.print(b2);
  Serial.print("  b3: ");
  Serial.print(b3);

  Serial.print("  Motor0: ");
  Serial.print(Motor0);
  Serial.print("  Motor1: ");
  Serial.print(Motor1);
  Serial.print("  RunWithoutProg: ");
  Serial.print(RunWithoutProg);
  Serial.print("  ProgRunning: ");
  Serial.print(ProgRunning);

  Serial.print("  a0: ");
  Serial.print(a0);
  Serial.print("  a1: ");
  Serial.println(a1);
}

void PrintProcessed()
{
  Serial.print("a0_speed: ");
  Serial.print(a0_speed*100);
  Serial.print("%");
  Serial.print("  a1_speed: ");
  Serial.print(a1_speed*100);
  Serial.print("%");

  Serial.print("  b_speed: ");
  Serial.println(b_speed);
}

void StopAll()
{
  stepper0.stop();
  stepper1.stop();

  digitalWrite(EN_PIN_0, HIGH);
  digitalWrite(EN_PIN_1, HIGH);
}

void loop()
{
  GetInputs();

  if (!(RunWithoutProg || ProgRunning)) // if prog not running and run without prog set to false
  {
    StopAll();
  }
  else
  {
    ProcessAnalog();

    if (b_speed == 0) // stop
    {
      StopAll();
    }
    else if (b_speed == 15) // retract
    {
      if (Motor0)
      {
        digitalWrite(EN_PIN_0, LOW);
        digitalWrite(DIR_PIN_0, BACKWARD_0);
        stepper0.play(Retract_Hz_0*a0_speed);
      }
      if (Motor1)
      {
        digitalWrite(EN_PIN_1, LOW);
        digitalWrite(DIR_PIN_1, BACKWARD_1);
        stepper1.play(Retract_Hz_1*a1_speed);
      }
    }
    else // extrude
    {
      if (Motor0)
      {
        digitalWrite(EN_PIN_0, LOW);
        digitalWrite(DIR_PIN_0, FORWARD_0);
        stepper0.play(b_speed*Hz_mult_0*a0_speed);
      }
      if (Motor1)
      {
        digitalWrite(EN_PIN_1, LOW);
        digitalWrite(DIR_PIN_1, FORWARD_1);
        stepper1.play(b_speed*Hz_mult_1*a1_speed);
      }      
    }

    if (!Motor0) // stop 0
    {
      stepper0.stop();
      digitalWrite(EN_PIN_0, HIGH);
    }

    if (!Motor1) // stop 1
    {
      stepper1.stop();
      digitalWrite(EN_PIN_1, HIGH);
    }
  }

}
