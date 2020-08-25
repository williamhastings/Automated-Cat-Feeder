// Stepper, and Driver combination is 28BYJ-48, and driver ULN2003AN
// The 28BYJ-48 stepper is a 32 step stepper motor, with a built in gear reduction of 64 resulting in 2048 step per revolution

#include <Stepper.h>

/* ---------------------------------------------- *\
    Global Definitions
  \* ---------------------------------------------- */

// Stepper PINOUT to UNO PINOUT
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// Button PINOUT
#define PURGE_BUTTON 2

// Rotary Encoder PINOUT
#define ROTARY_ENCODER_DT 5
#define ROTARY_ENCODER_CLK 4

// Auger Rotation MIN/MAX
#define MIN_AUGER_MULTIPLIER 1
#define MAX_AUGER_MULTIPLIER 100

/* ---------------------------------------------- *\
    Global Constants
  \* ---------------------------------------------- */
const float STEPS_PER_REVOLUTION = 32;
const float STEPPER_GEAR_REDUCTION = 64;
const float STEPS_PER_REVOLUTION_REDUCED = STEPS_PER_REVOLUTION * STEPPER_GEAR_REDUCTION;
const int MOTOR_SPEED = 700;
const int EIGTH_REVOLUTION = STEPS_PER_REVOLUTION_REDUCED / 8;

// Stepper instance with the revolutions, and PINOUTs for the UNO matching up to the In1, In2, In3, In4 on
// the ULN2003 stepper driver. PINOUT sequence 1-3-2-4
Stepper motor(STEPS_PER_REVOLUTION, IN1, IN3, IN2, IN4);

/* ---------------------------------------------- *\
    Global Variables
  \* ---------------------------------------------- */
int StepsPerFeeding = EIGTH_REVOLUTION;
int PurgeButtonState;
int PreviousEncoderCLKState;
int CurrentEncoderCLKState;
byte AugerRotationMultiplier = MIN_AUGER_MULTIPLIER;

/* ---------------------------------------------- *\
    Main Loop Function List
\* ---------------------------------------------- */
bool (*loopFunc[2]) ();

void setup() {
  // Initialize serial port for debug
  Serial.begin(9600);

  // Set PINOUTs for purge button
  pinMode(PURGE_BUTTON, INPUT);

  // Set PINOUTs for rotary encoder
  pinMode(ROTARY_ENCODER_DT, INPUT);
  pinMode(ROTARY_ENCODER_CLK, INPUT);

  // Get initial state of rotary encoder
  PreviousEncoderCLKState = GetEncoderCLKState();

  // Set stepper speed
  motor.setSpeed(MOTOR_SPEED);

  // Set Loop Functions
  loopFunc[0] = CheckForPurge;
  loopFunc[1] = CheckForRotaryEncoderMovement;
}

void loop() {
  // Now we want to loop through the array of bool function pointers calling each
  // function, and returning if one of the control loops is triggered.
  for (int i = 0; i < sizeof(loopFunc); i++) {
    if ((*loopFunc[i]) ()) {
      return;
    }
  }
}

bool CheckForPurge() {
  PurgeButtonState = GetPurgeButtonState();
  if (PurgeButtonState == HIGH) {
    // Enable Stepper for 1/8 turns to purge ogger screw
    motor.step(EIGTH_REVOLUTION * AugerRotationMultiplier);
    return true;
  }
  return false;
}

bool CheckForRotaryEncoderMovement() {
  CurrentEncoderCLKState = GetEncoderCLKState();
  if (CurrentEncoderCLKState != PreviousEncoderCLKState) {
    if (GetEncoderDTState() != CurrentEncoderCLKState) {
      if (++AugerRotationMultiplier > MAX_AUGER_MULTIPLIER) {
        AugerRotationMultiplier = MAX_AUGER_MULTIPLIER;
      }
    } else {
      if (--AugerRotationMultiplier < MIN_AUGER_MULTIPLIER) {
        AugerRotationMultiplier = MIN_AUGER_MULTIPLIER;
      }
    }
    PreviousEncoderCLKState = CurrentEncoderCLKState;
    PrintAugerRotationMultiplier(AugerRotationMultiplier);
    return true;
  }
  return false;
}

void PrintAugerRotationMultiplier(int multiplier) {
  Serial.print("Current auger multiplier: ");
  Serial.println(multiplier);
}

int GetEncoderCLKState() {
  return digitalRead(ROTARY_ENCODER_CLK);
}

int GetEncoderDTState() {
  return digitalRead(ROTARY_ENCODER_DT);
}

int GetPurgeButtonState() {
  return digitalRead(PURGE_BUTTON);
}
