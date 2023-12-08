void blynk_setup() {
  // Setup WiFi network
  // WiFi.config(device_ip, gateway_ip, subnet_mask);
  // WiFi.begin(ssid, pass);

  // Setup Blynk
  // Blynk.config(BLYNK_AUTH_TOKEN, "ny3.blynk.cloud", 80);
  Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ssid, wifi_pass, "blynk.cloud", 80);
  while (Blynk.connect() == false) {
    Off.Update();
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  //  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void ConnectWifi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WiFi.SSID());
  WiFi.persistent(false);  // May be able to set to true disables automatic saving of wifi credentials
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pass);
  // Blynk.connect();

  int t1 = millis();
  while (!WiFi.isConnected()) {  // was !Blynk.connected()
    int t2 = millis();
    delay(500);
    Serial.print(".");  
    if ((t2-t1) > wifiTimeout) {
      ESP.wdtDisable();
      while (true){};
    }
  }

  Serial.println("\nConnected to wifi");
  printWifiStatus();
}


void manageBlynkConnection2() {
  if (!WiFi.isConnected()) {    // changed from !Blynk.connected()
    Serial.println("Wifi Disconnected");
    ConnectWifi();
    Off.Update();
  } else {
    breathe.Update();
  }
}


void TriggerIFTTT(String key, String event_name) {
  //https://maker.ifttt.com/trigger/smoke_detected/with/key/ZOl-kiJkYQGJKLrC6TNHOjh3JOxtHXHL9ia7agmNjc
  String url_1 = "/trigger/"; 
  String url_2 = "/with/key/";
  String url = url_1 + event_name + url_2 + key;
  if (!clientS.connect(ifttthost, httpsPort)) {
    Serial.println("Trigger connection failed");
    return;
  } else {
  Serial.println("Sent Trigger Successfully!");
  clientS.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + ifttthost + "\r\n" +
               "User-Agent: IFTTTDetector\r\n" +
               "Connection: close\r\n\r\n");
  }
}

void SafetyCheck()
{
    if ((currenttime - safety_timer) >= (safety_interval * 1000) && (currenttime - safety_check_timer) >= (safety_check_int * 1000))
    {
        enable_stepper();
        home_stepper();
        disable_stepper();
        safety_check_timer = millis();
        TriggerIFTTT(Key, Safety_Event);
        Blynk.virtualWrite(V2, 0);
    }
}

void AddFirewood()
{
  // If output = max_count and the temperature is less than the setpoint say we need more firewood added
  if (Output == max_count && temp < _setpoint && temp <= (last_temp + 2)) {
    if (first_max == true) {
      first_max = false;
      add_firewood_timer = millis();
    }
    if ((currenttime - add_firewood_timer) >= (add_firewood_interval*1000*60)) {
      firewood_needed_time = (((double)currenttime - (double)add_firewood_timer - (double)add_firewood_interval*60000.0)/60000.0);
      if ((currenttime - firewood_print_timer) >= (firewood_print_interval*1000*60)) {
        firewood_print_timer = millis();
        if (firewood_notifications == 1) {
          // Blynk.notify("Woodstove has needed firewood added for " + String((currenttime - add_firewood_timer)/60000) + " minutes.");
          Blynk.virtualWrite(V14, 1);
          last_temp = temp;
        }
      }
    } else {
      firewood_needed_time = 0;
      Blynk.virtualWrite(V14, 0);
    }
  }
  else {
    first_max = true;
    firewood_needed_time = 0;
    firewood_print_timer = 0;
    last_temp = _setpoint;
    Blynk.virtualWrite(V14, 0);
  }
}

  // check reset button state
void CheckButton()
{
  // Detect if button Pressed
  if (digitalRead(reset_button_pin) == LOW) {
    // Check if for rising edge
    if (reset_button_state == HIGH) {
      Serial.println("Reset Button Pressed");
      reset_button_state = LOW;
      enable_stepper();
      home_stepper();
      move_stepper(1, Output);
      disable_stepper();
      home_time = millis();
    }
  } 
  else {
    if (reset_button_state == LOW) {
        reset_button_state = HIGH;
    }
  }
}
