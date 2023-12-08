
// Callback for each Temperature Update
void syncPins()
{
    Serial.println("Syncing virtual pins");
    Blynk.syncVirtual(V0, V4, V5, V6, V7, V8, V13);
}

BLYNK_WRITE(V0)
{
    // Update Current Temperature - Input to PID Controller
    temp = param.asLong();
    Blynk.virtualWrite(V2, (int)Output);
    if ((currenttime - home_time) >= (home_interval * 1000 * 60) || first_home == true) {
        enable_stepper();
        home_stepper();
        move_stepper(1, Output);
        // Originally did not have a disable stepper in here to smooth this out,
        // but this is needed to turn off the stepper during long periods when the damper is at 0.
        disable_stepper();
        home_time = millis();
        first_home = false;
    }
    Serial.print("\nCurrent Temperature is: ");
    Serial.println(Input);
    Serial.print("Current Setpoint is: ");
    Serial.println(Setpoint);
    Serial.print("Calculated Position is: ");
    Serial.println(Output);
    if (abs(Output - current_damper_pos) >= damper_position_threshold ||
        (Output == max_output && current_damper_pos != Output) ||
        (Output == min_output && current_damper_pos != Output))
    {
        // Post Damper Position to App
        // Blynk.virtualWrite(V2, (int)Output);
        Serial.print("Posted Damper Postion as: ");
        Serial.println(Output);
        enable_stepper();
        if (Output == min_output) {
            home_stepper();
        } else {
            move_stepper(1, Output);
        }
        disable_stepper();
        current_damper_pos = Output;
    }
    // Update Safety Timer with latest time of received Temperature Data
    safety_timer = millis();
    // Serial.println(safety_timer);
}

// Callback for Each Setpoint Update
BLYNK_WRITE(V4)
{
    _setpoint = param.asLong();
    Serial.print("Temperature Set Point is: ");
    Serial.println(_setpoint);
    // Blynk.virtualWrite(V4, _setpoint);
}

// Callback for each P Gain Update
BLYNK_WRITE(V5)
{
    Kp = param.asLong();
    myPID.SetTunings(Kp, Ki, Kd);
    Serial.print("Updated Kp Gain to: ");
    Serial.println(Kp);
}

// Callback for each I Gain Update
BLYNK_WRITE(V6)
{
    Ki = param.asFloat();
    myPID.SetTunings(Kp, Ki, Kd);
    Serial.print("Updated Ki Gain to: ");
    Serial.println(Ki);
}

// Callback for each Home Interval Update
BLYNK_WRITE(V7)
{
    home_interval = param.asFloat();
    Serial.print("Updated Home Interval to: ");
    Serial.println(home_interval);
}

// Callback for each D Gain Update
BLYNK_WRITE(V8)
{
    Kd = param.asFloat();
    myPID.SetTunings(Kp, Ki, Kd);
    Serial.print("Updated Kd Gain to: ");
    Serial.println(Kd);
}

// // Callback for Add firewood timer
// BLYNK_WRITE(V9)
// {
//   add_firewood_interval = param.asLong();
// }

// // Callback for Add firewood notification Interval
// BLYNK_WRITE(V10)
// {
//   firewood_print_interval = param.asLong();
// }

// Callback to turn on/off firewood notifications
// BLYNK_WRITE(V11)
// {
//   firewood_notifications = param.asInt();
// }

// // Push of the firewood needed time
// BLYNK_READ(V12)
// {
//   Blynk.virtualWrite(V12, firewood_needed_time);
// }

// Callback to set the maximum damper output
BLYNK_WRITE(V13)
{
    max_output = param.asLong();
    max_count = max_output;
    myPID.SetOutputLimits(min_output, max_output);
}