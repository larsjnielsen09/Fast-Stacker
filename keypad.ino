float Msteps_per_frame ()
{
  return (MM_PER_FRAME[g.i_mm_per_frame] / MM_PER_ROTATION) * MICROSTEPS_PER_ROTATION;
}


short Nframes ()
{
  return short(((float)(g.point2 - g.point1)) / g.msteps_per_frame) + 1;
}


void process_keypad()
/*
 All the keypad runtime stuff goes here
 */
{
  float speed;

  // Disabling the last comment line displaying after COMMENT_DELAY interval:
  if (g.comment_flag == 1 && g.t > g.t_comment + COMMENT_DELAY)
  {
    g.comment_flag == 0;
    if (g.moving == 0)
      display_current_position();
  }

  // Refreshing whole display regularly (only when not moving, as it is slow):
  if (g.moving == 0 && g.calibrate_warning == 0 && g.t - g.t_display > DISPLAY_REFRESH_TIME)
  {
    g.t_display = g.t;
    display_all("  ");
  }


  // ?? Ignore keypad during emergency breaking
  if (g.breaking == 1 || (g.calibrate == 3 && g.calibrate_warning == 0) || g.error > 1)
    return;

  if (g.stacker_mode == 1 && g.moving == 0)
  {
    // The flag means we just initiated stacking:
    g.start_stacking = 1;
    // Time when stacking was initiated:
    g.t0_stacking = g.t;
    g.frame_counter = 0;
    display_frame_counter();
    g.pos_to_shoot = g.pos_short_old;
    g.stacker_mode = 2;
  }


  // Reading a keypad key if any:
  char key = keypad.getKey();
  KeyState state = keypad.getState();


  // Experimental: trying to add some two-key combinations. Assuming the Ctrl key (#) is pressed first
  if ((keypad.key[0].kstate == PRESSED) && (keypad.key[0].kchar == '#') && keypad.key[1].stateChanged && (keypad.key[1].kstate == PRESSED))
    // Two-key commands (they all involve the "Ctrl" key - "#")
  {
    // ??? Not sure if the second key is always in [1]:
    switch (keypad.key[1].kchar)
    {
      case 'C': // Initiate a full calibration
        // Ignore if moving:
        if (g.moving == 1)
          break;
        g.calibrate = 3;
        g.calibrate_flag = 0;
        g.calibrate_warning = 1;
        g.calibrate_init = g.calibrate;
        // Displaying the calibrate warning:
        display_all("  ");
        break;

      case 'B':  // Initiate emergency breaking
        change_speed(0.0, 0);
        // This should be after change_speed(0.0):
        g.breaking = 1;
        letter_status("B ");
        g.calibrate = 0;
        g.calibrate_flag = 0;
        g.calibrate_warning = 0;
        g.calibrate_init = g.calibrate;
        break;

      case '2': // Save parameters to first memory bank
        g.reg1 = {g.i_n_shots, g.i_mm_per_frame, g.i_fps, g.point1, g.point2};
        EEPROM.put( ADDR_REG1, g.reg1);
        display_comment_line("Saved to Reg1 ");
        break;

      case '3': // Read parameters from first memory bank
        EEPROM.get( ADDR_REG1, g.reg1);
        g.i_n_shots = g.reg1.i_n_shots;
        g.i_mm_per_frame = g.reg1.i_mm_per_frame;
        g.i_fps = g.reg1.i_fps;
        g.point1 = g.reg1.point1;
        g.point2 = g.reg1.point2;
        g.msteps_per_frame = Msteps_per_frame();
        g.Nframes = Nframes();
        EEPROM.put( ADDR_I_N_SHOTS, g.i_n_shots);
        EEPROM.put( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
        EEPROM.put( ADDR_I_FPS, g.i_fps);
        EEPROM.put( ADDR_POINT1, g.point1);
        EEPROM.put( ADDR_POINT2, g.point2);
        display_all("  ");
        display_comment_line("Read from Reg1");
        break;

      case '5': // Save parameters to second memory bank
        g.reg2 = {g.i_n_shots, g.i_mm_per_frame, g.i_fps, g.point1, g.point2};
        EEPROM.put( ADDR_REG2, g.reg2);
        display_comment_line("Saved to Reg2 ");
        break;

      case '6': // Read parameters from second memory bank
        EEPROM.get( ADDR_REG2, g.reg2);
        g.i_n_shots = g.reg2.i_n_shots;
        g.i_mm_per_frame = g.reg2.i_mm_per_frame;
        g.i_fps = g.reg2.i_fps;
        g.point1 = g.reg2.point1;
        g.point2 = g.reg2.point2;
        g.msteps_per_frame = Msteps_per_frame();
        g.Nframes = Nframes();
        EEPROM.put( ADDR_I_N_SHOTS, g.i_n_shots);
        EEPROM.put( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
        EEPROM.put( ADDR_I_FPS, g.i_fps);
        EEPROM.put( ADDR_POINT1, g.point1);
        EEPROM.put( ADDR_POINT2, g.point2);
        display_all("  ");
        display_comment_line("Read from Reg2");
        break;

      case '8': // Save parameters to third memory bank
        g.reg3 = {g.i_n_shots, g.i_mm_per_frame, g.i_fps, g.point1, g.point2};
        EEPROM.put( ADDR_REG3, g.reg3);
        display_comment_line("Saved to Reg3 ");
        break;

      case '9': // Read parameters from third memory bank
        EEPROM.get( ADDR_REG3, g.reg3);
        g.i_n_shots = g.reg3.i_n_shots;
        g.i_mm_per_frame = g.reg3.i_mm_per_frame;
        g.i_fps = g.reg3.i_fps;
        g.point1 = g.reg3.point1;
        g.point2 = g.reg3.point2;
        g.msteps_per_frame = Msteps_per_frame();
        g.Nframes = Nframes();
        EEPROM.put( ADDR_I_N_SHOTS, g.i_n_shots);
        EEPROM.put( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
        EEPROM.put( ADDR_I_FPS, g.i_fps);
        EEPROM.put( ADDR_POINT1, g.point1);
        EEPROM.put( ADDR_POINT2, g.point2);
        display_all("  ");
        display_comment_line("Read from Reg3");
        break;

      case '4': // Backlighting control
        g.backlight++;
        if (g.backlight > 2)
          g.backlight = 0;
        set_backlight();
        break;

      case '*': // Factory reset
        factory_reset();
        g.calibrate_warning = 1;
        g.calibrate_init = g.calibrate;
        display_all("  ");
        break;

      case '7': // Manual camera shutter triggering
        // Setting the shutter on:
        digitalWrite(PIN_SHUTTER, HIGH);
        g.shutter_on = 1;
        g.t_shutter = g.t;
        break;

      case '1': // Rewind a single frame step (no shooting)
        // Required microsteps per frame:
        g.msteps_per_frame = Msteps_per_frame();
        go_to(g.pos - g.msteps_per_frame, SPEED_LIMIT);
        g.frame_counter--;
        display_frame_counter();
        break;

      case 'D': // Fast-forward a single frame step (no shooting)
        // Required microsteps per frame:
        g.msteps_per_frame = Msteps_per_frame();
        go_to(g.pos + g.msteps_per_frame, SPEED_LIMIT);
        g.frame_counter++;
        display_frame_counter();
        break;
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  else
    // Single-key commands
  {
    // Action is only needed if the kepad state changed since the last time,
    // or if one of the parameter change keys was pressed longer than T_KEY_LAG microseconds:
    if (state != g.state_old && (state == PRESSED || state == RELEASED) ||
        state == PRESSED && g.state_old == PRESSED && (g.key_old == '2' || g.key_old == '3' || g.key_old == '5'
            || g.key_old == '6' || g.key_old == '8' || g.key_old == '9') && g.t - g.t_key_pressed > T_KEY_LAG)
    {

      if (state == PRESSED)
      {
        if (g.state_old != PRESSED)
        {
          g.key_old = key;
          // Memorizing the time when a key was pressed:
          g.t_key_pressed = g.t;
          if (g.calibrate_warning == 1)
            // Any key pressed when calibrate_warning=1 will initiate calibration:
          {
            g.calibrate_warning = 0;
            display_all("  ");
            return;
          }
        }

        else
          // We are here when a change parameter key was pressed longer than T_KEY_LAG
        {
          if (g.N_repeats == 0)
            // Will be used for keys with repetition (parameter change keys):
          {
            g.t_last_repeat = g.t;
            g.N_repeats = 1;
          }
          // We repeat a paramet change key once the time since the last repeat is larger than T_KEY_REPEat:
          if (g.t - g.t_last_repeat > T_KEY_REPEAT)
          {
            g.N_repeats++;
            g.t_last_repeat = g.t;
            // ??? A trick, as "key" has only proper value when just pressed
            key = g.key_old;
          }
          else
            // Too early for a repeated key, so returning now:
            return;
        }

        // Keys interpretation depends on the stacker_mode:
        if (g.stacker_mode == 0)
          // Mode 0: default; rewinding etc.
        {
          // When error 1 (limiter on initially), the only commands accepted are rewind and fast forward:
          if (g.error == 1 && key != '1' && key != 'A')
            return;

          switch (key)
          {
            case '1':  // Rewinding
              g.direction = -1;
              motion_status();
              change_speed(-SPEED_LIMIT, 0);
              break;

            case 'A':  // Fast forwarding
              g.direction = 1;
              motion_status();
              change_speed(SPEED_LIMIT, (short)0);
              break;

            case '4':  // Set foreground point
              g.point1 = g.pos_short_old;
              if (g.points_byte == 0 || g.points_byte == 2)
                g.points_byte = g.points_byte + 1;
              g.msteps_per_frame = Msteps_per_frame();
              g.Nframes = Nframes();
              points_status();
              display_two_point_params();
              display_two_points();
              display_comment_line("  P1 was set  ");
              EEPROM.put( ADDR_POINT1, g.point1);
              EEPROM.put( ADDR_POINTS_BYTE, g.points_byte);
              break;

            case 'B':  // Set background point
              g.point2 = g.pos_short_old;
              if (g.points_byte == 0 || g.points_byte == 1)
                g.points_byte = g.points_byte + 2;
              g.msteps_per_frame = Msteps_per_frame();
              g.Nframes = Nframes();
              points_status();
              display_two_point_params();
              display_two_points();
              display_comment_line("  P2 was set  ");
              EEPROM.put( ADDR_POINT2, g.point2);
              EEPROM.put( ADDR_POINTS_BYTE, g.points_byte);
              break;

            case '7':  // Go to the foreground point
              go_to(g.point1, SPEED_LIMIT);
              display_comment_line(" Going to P1  ");
              break;

            case 'C':  // Go to the background point
              go_to(g.point2, SPEED_LIMIT);
              display_comment_line(" Going to P2  ");
              break;

            case '0': // Start shooting (2-point focus stacking)
              // Checking the correctness of point1/2
              if (g.point2 > g.point1 && g.point1 >= g.limit1 && g.point2 <= g.limit2)
              {
                // Using the simplest approach which will result the last shot to always slightly undershoot
                g.Nframes = Nframes();
                // Finding the closest point:
                // Need to compute delta separately because of the limitations of arduino abs():
                short delta = g.pos_short_old - g.point1;
                short d1 = (short)abs(delta);
                delta = g.pos_short_old - g.point2;
                short d2 = (short)abs(delta);
                if (d1 < d2)
                {
                  go_to(g.point1, SPEED_LIMIT);
                  g.starting_point = g.point1;
                  g.destination_point = g.point2;
                  g.stacking_direction = 1;
                }
                else
                {
                  go_to(g.point2, SPEED_LIMIT);
                  g.starting_point = g.point2;
                  g.destination_point = g.point1;
                  g.stacking_direction = -1;
                }
                g.stacker_mode = 1;
                display_comment_line("2-points stack");
              }
              else
              {
                // Should print error message
                display_comment_line("Bad 2 points! ");
              }
              break;

            case '*':  // Initiate one-point focus stacking backwards
              // Simplest workaround: ignore the command if currently moving
              // (better solution would be to stop first)
              if (!g.moving)
              {
                // The flag means we just initiated stacking:
                g.start_stacking = 1;
                // Time when stacking was initiated:
                g.t0_stacking = g.t;
                g.frame_counter = 0;
                display_frame_counter();
                g.pos_to_shoot = g.pos_short_old;
                g.starting_point = g.pos_short_old;
                g.stacking_direction = -1;
                g.stacker_mode = 3;
                display_comment_line("1-point stack ");
              }
              break;

            case 'D':  // Initiate one-point focus stacking forward
              if (!g.moving)
              {
                // The flag means we just initiated stacking:
                g.start_stacking = 1;
                // Time when stacking was initiated:
                g.t0_stacking = g.t;
                g.frame_counter = 0;
                display_frame_counter();
                g.pos_to_shoot = g.pos_short_old;
                g.starting_point = g.pos_short_old;
                g.stacking_direction = 1;
                g.stacker_mode = 3;
                display_comment_line("1-point stack ");
              }
              break;

            case '2':  // Decrease parameter n_shots
              if (g.i_n_shots > 0)
                g.i_n_shots--;
              else
                break;
              EEPROM.put( ADDR_I_N_SHOTS, g.i_n_shots);
              //!!!
              //            display_one_point_params();
              display_all("  ");
              break;

            case '3':  // Increase parameter n_shots
              if (g.i_n_shots < N_PARAMS - 1)
                g.i_n_shots++;
              else
                break;
              EEPROM.put( ADDR_I_N_SHOTS, g.i_n_shots);
              display_one_point_params();
              break;

            case '5':  // Decrease parameter mm_per_frame
              if (g.i_mm_per_frame > 0)
                g.i_mm_per_frame--;
              else
                break;
              // Required microsteps per frame:
              g.msteps_per_frame = Msteps_per_frame();
              // Using instead the simplest approach, which will result the last shot to always slightly undershoot
              g.Nframes = Nframes();
              display_all("  ");
              EEPROM.put( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
              break;

            case '6':  // Increase parameter mm_per_frame
              if (g.i_mm_per_frame < N_PARAMS - 1)
              {
                g.i_mm_per_frame++;
                // Estimating the required speed in microsteps per microsecond
                speed = SPEED_SCALE * FPS[g.i_fps] * MM_PER_FRAME[g.i_mm_per_frame];
                // Reverting back if required speed > maximum allowed:
                if (speed > SPEED_LIMIT)
                {
                  g.i_mm_per_frame--;
                  break;
                }
              }
              else
                break;
              // Required microsteps per frame:
              g.msteps_per_frame = Msteps_per_frame();
              // Using instead the simplest approach, which will result the last shot to always slightly undershoot
              g.Nframes = Nframes();
              EEPROM.put( ADDR_I_MM_PER_FRAME, g.i_mm_per_frame);
              display_all("  ");
              break;

            case '8':  // Decrease parameter fps
              if (g.i_fps > 0)
                g.i_fps--;
              else
                break;
              EEPROM.put( ADDR_I_FPS, g.i_fps);
              display_all("  ");
              break;

            case '9':  // Increase parameter fps
              if (g.i_fps < N_PARAMS - 1)
              {
                g.i_fps++;
                // Estimating the required speed in microsteps per microsecond
                speed = SPEED_SCALE * FPS[g.i_fps] * MM_PER_FRAME[g.i_mm_per_frame];
                // Reverting back if required speed > maximum allowed:
                if (speed > SPEED_LIMIT)
                {
                  g.i_fps--;
                  break;
                }
              }
              else
                break;
              EEPROM.put( ADDR_I_FPS, g.i_fps);
              display_all("  ");
              break;

          } // End of case
        }
        else if (g.stacker_mode > 0)
          // Mode 1/2: focus stacking
        {
          // Any key press in stacking mode interrupts stacking
          g.stacker_mode = 0;
          change_speed(0.0, 0);
          display_comment_line("Stacking abort");
        }

      }  // if (PRESSED)
      else
        // if a key was just released
      {
        // Resetting the counter of key repeats:
        g.N_repeats = 0;
        // Breaking / stopping if no keys pressed (only after rewind/fastforward)
        if ((g.key_old == '1' || g.key_old == 'A') && g.moving == 1 && state == RELEASED)
        {
          change_speed(0.0, 0);
        }
      }

      g.state_old = state;
    }  // End of if(keyStateChanged)
  } // End of two-key / one-key if


  return;
}

