/* Sergey Mashchenko 2015

   Stepper motor module

   To be used with automated macro rail for focus stacking
*/
#include <EEPROM.h>
#include "stacker.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void setup() {

  // Setting pins for EasyDriver to OUTPUT:
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_LIMITERS, INPUT_PULLUP);

  // Writing initial values to the motor pins:
  digitalWrite(PIN_ENABLE, HIGH); // Not using the holding torque feature (to save batteries)

  // Limiting switches should not be on when powering up:
  limit_on = digitalRead(PIN_LIMITERS);
  if (limit_on == HIGH)
  {
    // Give intsructions to power down arduino, remove the controller cable, and manually rewind
    // the focusing knob until the switch is off. This should follow by limiter calibration.
    abortMy = 1;
  }
  //!!!
  abortMy = 0;

  // Initializing program parameters:
  direction = 0;
  speed1 = 0.0;
  accel = 0;
  speed0 = 0.0;
  speed = 0.0;

  // Checking if EEPROM was never used:
  if (EEPROM.read(0) == 255 && EEPROM.read(1) == 255 && EEPROM.read(2) == 255 && EEPROM.read(3) == 255)
  {
    pos = 0.0;
    calibrate = 3;
    limit1 = 0;
    limit2 = 0;
  }
  else
  {
    // Reading the current position from EEPROM:
    EEPROM.get( ADDR_POS, pos );
    EEPROM.get( ADDR_CALIBRATE, calibrate );
    EEPROM.get( ADDR_LIMIT1, limit1);
    EEPROM.get( ADDR_LIMIT2, limit2);
  }
  calibrate_init = calibrate;
  pos0 = pos;
  pos_short_old = floor(pos);
  t0 = micros();
  t = t0;
  breaking = 0;

  // For testing:
  digitalWrite(PIN_ENABLE, LOW);        
  flag = 0;
  pos = 0;
//  Serial.begin(9600);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop()
{

  // Simple test:

  // At t=1s, start moving forward with constant acceleration
  if (flag == 0 && direction == 0)
  {
    flag = 1;
    // ACcelerate to target speed in -1 direction:
    accel_change(1, 1, SPEED_LIMIT);
    pos0 = 0.0;
    show_params();
  }

  // After 3s of moving forward, start slowing down with constant deceleration
  if (flag == 1 && accel == 0 && t - t0 > 3000000)
  {
    flag = 2;
    // Stop (valid for any direction):
    accel_change(0, -1, 0.0);
    show_params();
  }

  if (flag == 2 && direction == 0)
  {
    flag = 3;
    // Accelerate in the opposite direction:
    accel_change(-1, 1, SPEED_LIMIT);
    show_params();
  }

  if (flag == 3 && accel == 0 && t - t0 > 3000000)
  {
    flag = 4;
// Stop:
    accel_change(0, -1, 0.0);
    show_params();
  }

  // Got to the start:
  if (flag == 4 && direction == 0)
  {
    flag = 0;
  }

  // All the processing related to the two extreme limits for the macro rail movements:
  //  limiters();

  // Prevent motor operations if limiters are engaged initially:
  //  if (abortMy && direction == 0)
  //    return;

  // Perform calibration of the limiters if requested (only when the rail is at rest):
  //  if (calibrate_init>0 && direction==0 && breaking==0)
  //    calibration();

  // Issuing write to stepper motor driver pins if/when needed:
  motor_control();

}

