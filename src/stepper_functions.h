void enable_stepper()
{
    digitalWrite(EN, LOW);
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, HIGH);
    //Serial.println("Stepper Enabled");
    delay(500);
}

void disable_stepper()
{
    digitalWrite(EN, HIGH);
    //Serial.println("Stepper Disabled");
    delay(100);
}

void move_stepper(long speed, long target)
{
    speed = 1024 - speed;
    bool direction;
    if (target > count)
    {
        // Move outward
        direction = HIGH;
    }
    else
    {
        // Move inward
        direction = LOW;
    }
    digitalWrite(DIR, direction);
    while (count != target)
    {
        // Endstop is pressed
        if (digitalRead(endstop) == LOW)
        {
            count = 0;
            if (target > count)
            {
                direction = HIGH;
                digitalWrite(DIR, direction);
            }
            else if (target == 0)
            {
                return;
            }
        }
        digitalWrite(STP, HIGH);
        delayMicroseconds(speed);
        digitalWrite(STP, LOW);
        delayMicroseconds(speed);
        if (direction == HIGH)
        {
            count++;
        }
        else
        {
            count--;
        }
        ESP.wdtFeed();
    }
    Serial.print("Count is: ");
    Serial.println(count);
    delay(100);
}

void home_stepper()
{
    int home_speed = 600;
    Serial.println("Homing Stepper");
    digitalWrite(DIR, LOW);
    while (digitalRead(endstop) == HIGH)
    {
        digitalWrite(STP, HIGH);
        delayMicroseconds(home_speed);
        digitalWrite(STP, LOW);
        delayMicroseconds(home_speed);
        count--;
        ESP.wdtFeed();
    }
    long count_error = count - 0;
    count = 0;
    Serial.print("Count Error was: ");
    Serial.println(count_error);
    Serial.println("Count Reset to 0");
    Blynk.virtualWrite(V3, count_error);
}