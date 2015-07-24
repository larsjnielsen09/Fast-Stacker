//#include <Arduino.h>
//#include "stacker.h"

void motor_control()
/* Controlling the stepper motor, based on current time, direction, target speed (speed1), acceleration (accel),
   values at the last accel change (t0, pos0, speed0), and old integer position pos_old_short.

   Important: "direction" can be set to zero only here! Also, it should be set to -1 or 1 only outside of this function.
   "direction" cannot go straight from "-1" to "1" and visa versa without going through "0" first.
 */
{
  unsigned long dt, dt_a;
  float dV;
  short new_accel;

  // Current time in microseconds:
  t = micros();

  // direction=0 means no motion, so simply returning:
  if (direction == 0)
    return;


  ////////   PART 1: estimating the current position, pos

  // Time in microseconds since the last accel change:
  dt = t - t0;
  // Storing the current accel value:
  new_accel = accel;

//!!!
//    dt_a=0;
//    dV=0;
//    float p1, p2, p3, pp;

  if (accel != 0)
    // Accelerating/decelerating cases
  {
    // Change of speed (assuming accel hasn't changed from t0 to t);
    // can be negative or positive:
    dV = (float)accel * ACCEL_LIMIT * (float)dt;

    // Current speed (should be always >0):
    speed = speed0 + dV;

    // If going beyond the target speed, stop accelerating:
    if ((accel == 1 && speed >= speed1) || (accel == -1 && speed <= speed1))
    {
      // t_a : time in the past (between t0 and t) when acceleration should have changed to 0, to prevent going beyong the target speed
      // dt_a = t_a-t0; should be >0, and <dt:
      dt_a = (float)accel * (speed1 - speed0) / ACCEL_LIMIT;
      // Current position has two components: first one (from t0 to t_a) is still accelerated,
      // second one (t_a ... t) has accel=0:
      pos = pos0 + (float)direction * ((float)dt_a * (speed0 + 0.5 * (float)accel * ACCEL_LIMIT * (float)dt_a) + speed1*(float)(dt - dt_a));
      //p1 = dt_a * direction * speed0;
      //p2 = dt_a*0.5 * direction * accel * ACCEL_LIMIT * dt_a;
      //p3 = speed1*(float)(dt - dt_a);
      //pp=p1+p2+p3;
      speed = speed1;
      new_accel = 0;
      if (accel == -1)
      {
        // At this point we stopped, so no need to revisit the motor_control module:
        direction = 0;
        // We can lower the breaking flag now, as we already stopped:
        breaking = 0;
      }
    }
    else
    {
      // Current position when accel didn't change between t0 and t:
      pos = pos0 +  (float)direction * (float)dt * (speed0 + 0.5 * dV );
    }
  }
  else
  {
    // Current position when accel=0
    pos = pos0 +  (float)dt * (float)direction * speed0;
  }

  //////////  PART 2: Estimating if we need to make a step, and making the step if needed


  // Integer position (in microsteps):
  short pos_short = floor(pos);
/*
Serial.print("pos=");
Serial.print(pos);
Serial.print(" pos_short=");
Serial.print(pos_short);
Serial.print(" new_accel=");
Serial.print(new_accel);
Serial.print(" dir=");
Serial.print(direction);
Serial.print(" speed1=");
Serial.print(speed1,6);
Serial.print(" pos0=");
Serial.print(pos0);
Serial.print(" dt=");
Serial.print(dt);
Serial.print(" p1=");
Serial.print(p1);
Serial.print(" p2=");
Serial.print(p2);
Serial.print(" p3=");
Serial.print(p3);
Serial.print(" pp=");
Serial.print(pp);
Serial.print(" dt_a=");
Serial.print(dt_a);
Serial.print(" acc_limit=");
Serial.print(ACCEL_LIMIT,9);
Serial.print(" dV=");
Serial.println(dV,6);
delay(100);
*/
  // If the pos_short changed since the last step, do another step
  if (pos_short != pos_short_old)
  {
    // One microstep (driver direction pin should have been written to elsewhere):
    digitalWrite(PIN_STEP, LOW);
    // For Easydriver, the delay should be at least 1.0 us:
    delayMicroseconds(STEP_LOW_DT);
    digitalWrite(PIN_STEP, HIGH);

    // Saving the current position as old:
    pos_short_old = pos_short;
  }


  //////////  PART 3: Finalizing

  // If accel was modified here, update pos0, t0 to the current ones:
  if (new_accel != accel)
  {
    t0 = t;
    pos0 = pos;
    //?
    speed0 = speed;
    accel = new_accel;
  }


  return;
}


