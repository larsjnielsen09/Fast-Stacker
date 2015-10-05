/* Sergey Mashchenko 2015

   Automated macro rail for focus stacking

   The hardware used:
    - Velbon Mag Slider manual macro rail, with some customization
    - Ultrathin 2-Phase 4-Wire 42 Stepper Motor 1.8 Degree 0.7A (http://www.ebay.ca/itm/Ultrathin-2-Phase-4-Wire-42-Stepper-Motor-1-8-Degree-0-7A-e-/261826922341?pt=LH_DefaultDomain_0&hash=item3cf619c765 )
    - Arduino Uno R3
    - EasyDriver stepping Stepper Motor Driver V4.4
    - 4x4 keys keypad (http://www.ebay.ca/itm/4x4-Matrix-high-quality-Keyboard-Keypad-Use-Key-PIC-AVR-Stamp-Sml-/141687830020?pt=LH_DefaultDomain_0&hash=item20fd40b604 )
    - Nokia 5110 LCD display + four 10 kOhm resistors + 1 kOhm + 330 Ohm (https://learn.sparkfun.com/tutorials/graphic-lcd-hookup-guide)
    - 5V Relay SIP-1A05 + 1N4004 diode + 33 Ohm resistor; to operate camera shutter (http://www.forward.com.au/pfod/HomeAutomation/OnOffAddRelay/index.html)

   I am using the following libraries:

    - pcd8544 (for Nokia 5110): https://github.com/snigelen/pcd8544
    - Keypad library: http://playground.arduino.cc/Code/Keypad

*/
#include <EEPROM.h>
#include <math.h>
#include <Keypad.h>
#include "pcd8544.h"
#include "stacker.h"
#include "stdio.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void factory_reset()
{
  g.pos = 0.0;
  g.calibrate = 3;
  g.limit1 = -30000;
  g.limit2 = 30000;
  g.i_n_shots = 9;
  g.i_mm_per_frame = 5;
  g.i_fps = 16;
  g.point1 = -3000;
  g.point2 = 3000;
  g.points_byte = 0;
  g.backlight = 2;
  g.reg1 = {g.i_n_shots, g.i_mm_per_frame, g.i_fps, g.point1, g.point2};
  g.reg2 = g.reg1;
  g.reg3 = g.reg1;
  // Saving these values in EEPROM:
  EEPROM.put( ADDR_POS, g.pos );
  EEPROM.put( ADDR_CALIBRATE, g.calibrate );
  EEPROM.put( ADDR_LIMIT1, g.limit1);
  EEPROM.put( ADDR_LIMIT2, g.limit2);
  EEPROM.put( ADDR_I_N_SHOTS, g.i_n_shots);
  EEPROM.put( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
  EEPROM.put( ADDR_I_FPS, g.i_fps);
  EEPROM.put( ADDR_POINT1, g.point1);
  EEPROM.put( ADDR_POINT2, g.point2);
  EEPROM.put( ADDR_POINTS_BYTE, g.points_byte);
  EEPROM.put( ADDR_BACKLIGHT, g.backlight);
  EEPROM.put( ADDR_REG1, g.reg1);
  EEPROM.put( ADDR_REG2, g.reg2);
  EEPROM.put( ADDR_REG3, g.reg3);
  return;
}

void setup() {

  g.error = 0;
  g.calibrate_warning = 0;

  // Setting pins for EasyDriver to OUTPUT:
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_LIMITERS, INPUT_PULLUP);

  pinMode(PIN_SHUTTER, OUTPUT);

  pinMode(PIN_LCD_LED, OUTPUT);

#ifdef DEBUG
  Serial.begin(250000);
  delay(250);
#endif

#ifdef LCD
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
#endif
  //  delay(500);

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
  keypad.setDebounceTime(50);
  g.key_old = '=';

  // Limiting switches should not be on when powering up:
  unsigned char limit_on = digitalRead(PIN_LIMITERS);
  if (limit_on == HIGH)
  {
    g.error = 1;
  }

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
  g.comment_flag = 0;

  // Uncomment to emulate the very first run:
  //  EEPROM.write(0, 255);  EEPROM.write(1, 255);

  // Checking if EEPROM was never used:
  if (EEPROM.read(0) == 255 && EEPROM.read(1) == 255)
  {
    // Values for the very first run:
    factory_reset();
  }
  else
  {
    // Reading the values from EEPROM:
    EEPROM.get( ADDR_POS, g.pos );
    EEPROM.get( ADDR_CALIBRATE, g.calibrate );
    EEPROM.get( ADDR_LIMIT1, g.limit1);
    EEPROM.get( ADDR_LIMIT2, g.limit2);
    EEPROM.get( ADDR_I_N_SHOTS, g.i_n_shots);
    EEPROM.get( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
    EEPROM.get( ADDR_I_FPS, g.i_fps);
    EEPROM.get( ADDR_POINT1, g.point1);
    EEPROM.get( ADDR_POINT2, g.point2);
    EEPROM.get( ADDR_POINTS_BYTE, g.points_byte);
    EEPROM.get( ADDR_BACKLIGHT, g.backlight);
    EEPROM.get( ADDR_REG1, g.reg1);
    EEPROM.get( ADDR_REG2, g.reg2);
    EEPROM.get( ADDR_REG3, g.reg3);
#ifdef DEBUG
    Serial.println("EEPROM values:");
    Serial.println(g.pos, 2);
    Serial.println(g.calibrate);
    Serial.println(g.limit1);
    Serial.println(g.limit2);
    Serial.println(g.i_n_shots);
    Serial.println(g.i_mm_per_frame);
    Serial.println(g.i_fps);
    Serial.println(g.point1);
    Serial.println(g.point2);
    Serial.println(g.points_byte);
#endif
  }

  set_backlight();

  g.calibrate_flag = 0;
  if (g.calibrate == 3)
    g.calibrate_warning = 1;
  // Memorizing the initial value of g.calibrate:
  g.calibrate_init = g.calibrate;
  g.pos0 = g.pos;
  g.pos_short_old = floorMy(g.pos);
  g.t0 = micros();
  g.t = g.t0;
  g.t_key_pressed = g.t0;
  g.t_last_repeat = g.t0;
  g.N_repeats = 0;
  g.breaking = 0;
  g.pos_stop_flag = 0;
  g.frame_counter = 0;
  g.state_old = (KeyState)0;

  g.msteps_per_frame = Msteps_per_frame();
  g.Nframes = Nframes();

  // Default lcd layout:
#ifdef LCD
  lcd.clear();
#endif
  display_all(" ");

#ifdef TIMING
  g.t_old = g.t0;
  g.i_timing = 0;
#endif

#ifdef DEBUG
  Serial.print("Initial g.limit1=");
  Serial.println(g.limit1);
#endif
#ifdef MOTOR_DEBUG
  g.calibrate = 0;
#endif

}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop()
{

  // Processing the keypad:
  process_keypad();

  // All the processing related to the two extreme limits for the macro rail movements:
#ifndef MOTOR_DEBUG
  limiters();
#endif

  // Perform calibration of the limiters if requested (only when the rail is at rest):
  calibration();

  // Camera shutter control:
  camera();

  // Issuing write to stepper motor driver pins if/when needed:
  motor_control();

#ifdef TIMING
  timing();
#endif

}

