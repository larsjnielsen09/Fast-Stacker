void process_keypad()
/*
 All the keypad runtime stuff goes here
 */
{
  // Action is only needed if the kepad state changed since the last time:
  if (keypad.keyStateChanged())
  {
    // Reading a keypad key if any:
    char key = keypad.getKey();

    // Keys interpretation depends on the stacker_mode:
    if (g.stacker_mode == 0)
      // Mode 0: default; rewinding etc.
    {
      switch (key)
      {
        case NO_KEY: // Breaking / stopping if no keys pressed (only after rewind/fastforward)
          if ((g.key_old == '1' || g.key_old == 'A') && g.moving == 1)
            change_speed(0.0, 0);
        case '1':  // Rewinding
          change_speed(-SPEED_LIMIT, 0);
          break;
        case 'A':  // Fast forwarding
          change_speed(SPEED_LIMIT, 0);
          break;
        case '6':  // Set foreground point
          g.point1 = g.pos_short_old;
          break;
        case 'B':  // Set background point
          g.point2 = g.pos_short_old;
          break;
        case '7':  // Go to the foreground point
          go_to(g.point1,SPEED_LIMIT);
          break;
        case 'C':  // Go to the background point
          go_to(g.point2,SPEED_LIMIT);
          break;
        case 'D': // Start shooting (2-point focus stacking)
          // Checking the correctness of limit1/2
          if (g.point2 > g.point1 && g.point1 >= g.limit1 && g.point2 <= g.limit2)
          {
            // Adjusting points to fit an integer number of shots (by slightly extending or shrinking the position range)
            // Required microsteps per frame:
            g.msteps_per_frame = g.mm_per_frame / MM_PER_ROTATION * MICROSTEPS_PER_ROTATION;
            // Number of frames rounded to the nearest integer:
            g.Nframes = nintMy(((float)(g.point2-g.point1))/g.msteps_per_frame) + 1;
            // Adjusted distance to travel:
            float d = ((float)(g.Nframes-1))*g.msteps_per_frame;
            // Original midpoint:
            float mid = ((float)g.point1+(float)g.point2)/2.0;
            // Adjusted points 
            g.point1 = (short)(mid - d/2.0);
            // Making sure we don't go beyond the limits:
            if (g.point1 < g.limit1)
              g.point1 = g.limit1;
            g.point2 = g.point1 + nintMy(d);
            if (g.point2 > g.limit2)
              g.point2 = g.limit2;
            // Finding the closest point:
            short d1 = (short)fabs(g.pos_short_old - g.point1);
            short d2 = (short)fabs(g.pos_short_old - g.point2);
            if (d1 < d2)
            {
              go_to(g.point1,SPEED_LIMIT);
              g.first_point = g.point1;
              g.second_point = g.point2;
              g.stacking_direction = 1;
            }
            else
            {
              go_to(g.point2,SPEED_LIMIT);
              g.first_point = g.point2;
              g.second_point = g.point1;
              g.stacking_direction = -1;
            }
            g.stacker_mode = 1;
          }
      }
    }
    else if (g.stacker_mode > 0)
      // Mode 1/2: focus stacking
    {
      // Any key press in stacking mode interrupts stacking
      g.stacker_mode = 0;
      change_speed(0.0, 0);
    }


    g.key_old = key;
  }

  // Stuff to do at every call to keypad()
  if (g.stacker_mode == 1 && g.moving==0)
  {
    // Estimating the required speed in microsteps per microsecond
    float speed = SPEED_SCALE * g.fps * g.mm_per_frame;
    go_to(g.second_point,speed);
    g.frame_counter = 0;
    g.pos_to_shoot = g.pos_short_old;
    g.stacker_mode = 2;
  }

  return;
}
