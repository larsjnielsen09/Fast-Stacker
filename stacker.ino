/* Sergey Mashchenko 2015

   Stepper motor module

   To be used with automated macro rail for focus stacking
*/
#include <EEPROM.h>
#include <math.h>
#include <Keypad.h>
#include "pcd8544.h"
#include "stacker.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void setup() {

  // Setting pins for EasyDriver to OUTPUT:
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_LIMITERS, INPUT_PULLUP);

  pinMode(PIN_SHUTTER, OUTPUT);

  pinMode(PIN_LCD_LED, OUTPUT);
  // Change the LCD backlighting here (0...255). WIll be implemented as user-controlled later
  analogWrite(PIN_LCD_LED, 255);

  lcd.begin();  // Always call lcd.begin() first.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("  Automated ");
  lcd.println("focus stacker");
  lcd.print(" ver. ");
  lcd.println(VERSION);
  lcd.println("(c) Sergey");
  lcd.println("Mashchenko");
  lcd.print("  2015");
  delay(500);

  // Writing initial values to the motor pins:
#ifdef SAVE_ENERGY
  digitalWrite(PIN_ENABLE, HIGH); // Not using the holding torque feature (to save batteries)
#else
  digitalWrite(PIN_ENABLE, LOW); // Using the holding torque feature (bad for batteries; good for holding torque and accuracy)
#endif

  digitalWrite(PIN_SHUTTER, LOW);

  // Keypad stuff:
  // No locking for keys:
  keypad.setHoldTime(1000000);
  g.key_old = '=';

  // Limiting switches should not be on when powering up:
  g.limit_on = digitalRead(PIN_LIMITERS);
  if (g.limit_on == HIGH)
  {
    // Give intsructions to power down arduino, remove the controller cable, and manually rewind
    // the focusing knob until the switch is off. This should follow by limiter calibration.
    g.abortMy = 1;
  }
  //!!!
  g.abortMy = 0;

  // Initializing program parameters:
  g.moving = 0;
  g.speed1 = 0.0;
  g.accel = 0;
  g.speed0 = 0.0;
  g.speed = 0.0;
  g.pos_stop_flag = 0;
  g.stacker_mode = 0;
  g.shutter_on = 0;
  g.direction = 1;
  g.points_byte = 0;

  // Checking if EEPROM was never used:
  if (EEPROM.read(0) == 255 && EEPROM.read(1) == 255 && EEPROM.read(2) == 255 && EEPROM.read(3) == 255)
  {
    g.pos = 0.0;
    g.calibrate = 3;
    g.limit1 = 0;
    g.limit2 = 0;
    g.i_n_shots = 0;
    g.i_mm_per_frame = 0;
    g.i_fps = 0;
  }
  else
  {
    // Reading the current position from EEPROM:
    EEPROM.get( ADDR_POS, g.pos );
    EEPROM.get( ADDR_CALIBRATE, g.calibrate );
    EEPROM.get( ADDR_LIMIT1, g.limit1);
    EEPROM.get( ADDR_LIMIT2, g.limit2);
    EEPROM.get( ADDR_I_N_SHOTS, g.i_n_shots);
    EEPROM.get( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
    EEPROM.get( ADDR_I_FPS, g.i_fps);
  }
  g.calibrate_init = g.calibrate;
  g.calibrate_flag = 0;
  g.pos0 = g.pos;
  g.pos_short_old = floorMy(g.pos);
  g.t0 = micros();
  g.t = g.t0;
  g.breaking = 0;
  g.pos_stop_flag = 0;
  g.frame_counter = 0;

  // Default lcd layout:
  lcd.clear();
  display_all(" ");
    
  // Testing:
  g.flag = 0;
  g.pos = 0;
  g.point1 = 0;
  g.point2 = 10000;
#ifdef DEBUG
  Serial.begin(9600);
#endif
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop()
{

/*
  // Simple test:

  // At t=0, start moving forward with constant acceleration
  if (g.flag == 0)
  {
    g.flag = 1;
    // ACcelerate to positive speed:
    change_speed(SPEED_LIMIT / 3.0, 0);
    //    pos0 = 0.0;
    show_params();
  }

  if (g.flag == 1 && g.accel == 0 && g.t - g.t0 > 2000000)
  {
    g.flag = 2;
    change_speed(SPEED_LIMIT, 0);
    show_params();
  }

  if (g.flag == 2 && g.accel == 0 && g.t - g.t0 > 2000000)
  {
    g.flag = 3;
    change_speed(-SPEED_LIMIT / 3.0, 0);
    show_params();
  }

  if (g.flag == 3 && g.accel == 0 && g.t - g.t0 > 2000000)
  {
    g.flag = 4;
    change_speed(-SPEED_LIMIT, 0);
    show_params();
  }

  // Go to the start:
  if (g.flag == 4 && g.accel == 0 && g.t - g.t0 > 2000000)
  {
    g.flag = 0;
  }
  */

  // Processing the keypad:
  process_keypad();

  // All the processing related to the two extreme limits for the macro rail movements:
  if (g.moving == 1 && g.breaking == 0)
    limiters();

  // Prevent motor operations if limiters are engaged initially:
  //  if (abortMy && direction == 0)
  //    return;

  // Perform calibration of the limiters if requested (only when the rail is at rest):
  if (g.calibrate_init > 0 && g.moving == 0 && g.breaking == 0)
    calibration();

  // Camera shutter control:
  camera();

  // Issuing write to stepper motor driver pins if/when needed:
  motor_control();

}

