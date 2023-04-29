#include <WiFiClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SpeedyStepper.h> // https://github.com/Stan-Reifel/SpeedyStepper.git Linux users: this library has an import with incorrect casing that will fail on Linux. Downloading from the repository rather than throguh Arduino Library Manager will fix the problem.
#include <WiFiManager.h>   //https://github.com/tzapu/WiFiManager

// Unused pin declarations (these pins are present on Motorized Macro Slider PCB V1.2):
// const int LIMIT1_PIN = 12;
// const int LIMIT2_PIN = 13;
// const int ENC_A_PIN = 32;
// const int ENC_B_PIN = 35;
// const int STEP_INDEX_PIN = 25;
// HardwareSerial &serial_stream = Serial2; //Serial2: RX pin 16, TX pin 17

// Pin declarations
const int SHUTTER_PIN = 5;
const int MOTOR_STEP_PIN = 18;
const int MOTOR_DIRECTION_PIN = 19;
const int STEP_EN_PIN = 26;
const int STEP_MS1_PIN = 27;
const int STEP_MS2_PIN = 14;
const int STEP_DIAG_PIN = 33;
const int LED_PWM_PIN = 4;
const uint8_t LED_CHANNEL = 0;

// Adjustable settings (through web interface)
float startPoint = 0; //mm
float endPoint = 0; //mm
float jogIncrement = 0.1; //mm
int numImages = 200;

// Adjustable settings (through web interface) that will be stored in EEPROM between power-cycles
Preferences preferences; //Used to store user-settings in EEPROM
float shootingSpeed = 0.1; //mm/s
float jogSpeed = 1; //mm/s
float deshakeDelay = 4; //s
float shutterDelay = 0.5; //s (used to prevent camera moves while the shutter is still open)
float overshootDistance = 0.5; //mm (used by goToStartPoint() to counteract mechanical backlash by making sure gear teeth are active from the beginning.)

//Non adjustable settings
float triggerDelay = 0.25; //s
const bool invert_directions = true; // Use this variable to swap behavior of forwards- and backwards-buttons (instead of rewiring motor)
const uint MAX_LED_BRIGHTNESS = 255;
const char* localDomainName = "OpenMacroRail"; // This enables http://OpenMacroRail.local (if your router supports it).

//Calculated settings:
const int invert_direction_multiplier = invert_directions ? -1 : 1; // Do not modify! This variable changes direction of moves according to the boolean invert_directions. 
float currentPos = 0.0; //mm
float distance = 0.0; //mm
float increment = 0.0; //mm
float remainingShootingTime = 0.0; //min
float totalShootingTime = 0.0; //min
int picsTaken = 0; 
int remainingPictures = 0; 
bool ledState = false;
int ledBrightness = 10;

WiFiManager wifiManager;
SpeedyStepper stepper;

//Motor and mechanical parameters
const float MOTOR_STEP_ANGLE = 1.8; // Degrees revolution per full step (motor parameter)
const uint32_t FULLSTEPS_PER_REVOLUTION = 360.0/MOTOR_STEP_ANGLE; // Number of steps per full revolution. 360/1.8 = 200
const uint32_t GEAR_REDUCTION_FACTOR = 2; // Mechanical reduction between motor and slider. 2:1 default
uint16_t microsteps = 8;                  //Default value 8 microsteps
uint32_t microstepsPerMillimeter = microsteps * FULLSTEPS_PER_REVOLUTION * GEAR_REDUCTION_FACTOR; // 8x microstepping, 200 steps per revolution, and 2:1 gear reduction gives 3200

//Flags
volatile bool JOGFWDFLAG = false;
volatile bool JOGBWDFLAG = false;

enum OPERATING_STATE{
  JOGGING,
  PHOTO_SESSION,
  EMERGENCY_STOP,
  BACKLASH_SWEEP,
  BACKLASH_DETECTED,
  HOMING,
  HOME_DETECTED,
  TEST_MODE
};
OPERATING_STATE current_operating_state = JOGGING;

enum SESSION_STATE{
  NEXT_IMAGE,
  WAIT_DESHAKE,
  WAIT_TRIGGER,
  WAIT_SHUTTER
};
SESSION_STATE current_session_state = NEXT_IMAGE;
long time_prev_session_state_change = millis();
long timeout_period = deshakeDelay; //This variable adapts to current_session_state, so it's not constant!

WebServer server(80);

const char *webpage = 
#include "webpage.h"
;

void handleRoot() {

  server.send(200, "text/html", webpage);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void jog_fwd(){
  JOGFWDFLAG = false;
  stepper.moveRelativeInMillimeters(jogIncrement * invert_direction_multiplier);
  currentPos += jogIncrement;
}

void jog_bwd(){
  JOGBWDFLAG = false;
  stepper.moveRelativeInMillimeters(-jogIncrement  * invert_direction_multiplier);
  currentPos -= jogIncrement;
}

void activateCameraTrigger(){
  //NOTE: the shutter pin is pulled high to activate the optocoupler, which in turn pulls the camera trigger signal low.
  digitalWrite(SHUTTER_PIN, HIGH); 
}

void deactivateCameraTrigger(){
  digitalWrite(SHUTTER_PIN, LOW);
  picsTaken +=1; //Note: The picture is actually taken in the "activateCameraTrigger" function, but we increment when the wait is over to reduce risk of constant triggering.
}

void toggleCameraTrigger(){
  // It is strongly discouraged to use this function during normal operation as it will block the website from being responsive.
  activateCameraTrigger();
  delay(triggerDelay*1000);
  deactivateCameraTrigger();
}

void goToStartPoint(){
  float currentMove;
  //TODO: Manual overshoot should be replaced with load sensing through TMC SafeGuard function.
  if((currentPos - startPoint) < 0){
    currentMove = (-(currentPos - startPoint) + overshootDistance);
    stepper.moveRelativeInMillimeters(currentMove * invert_direction_multiplier);
    currentPos += currentMove;
    stepper.moveRelativeInMillimeters(-overshootDistance * invert_direction_multiplier);
    currentPos -= overshootDistance;
  }else{
    currentMove = (-(currentPos - startPoint) - overshootDistance);
    stepper.moveRelativeInMillimeters(currentMove * invert_direction_multiplier);
    currentPos += currentMove;
    stepper.moveRelativeInMillimeters(overshootDistance * invert_direction_multiplier);
    currentPos += overshootDistance;
  }
}

void calculateStats(){
  increment = (distance / (numImages - 1));
  remainingPictures = numImages - picsTaken;
  if(distance != 0){
    totalShootingTime = (numImages * (shutterDelay + deshakeDelay + 0.2) + distance / shootingSpeed) / 60; //Simplified calculation that ignores accelleration
  }else{
    totalShootingTime = 0;
  }
  if(current_operating_state==OPERATING_STATE::PHOTO_SESSION){
    remainingShootingTime = (remainingPictures * (shutterDelay + deshakeDelay + 0.2) + distance / shootingSpeed) / 60; //Simplified calculation that ignores accelleration
  }else{
    remainingShootingTime = 0;
  }
}

void setStartPoint(){
  if (current_operating_state != PHOTO_SESSION)
  {
    startPoint = currentPos;
    distance = endPoint-startPoint;
    calculateStats();
  }
}

void setEndPoint(){
  if (current_operating_state != PHOTO_SESSION)
  {
    endPoint = currentPos;
    distance = endPoint-startPoint;
    calculateStats();
  }
}

void updateStepperSettings(){
  stepper.setStepsPerMillimeter(microstepsPerMillimeter);
  if(current_operating_state==JOGGING){
    stepper.setSpeedInMillimetersPerSecond(jogSpeed);
  }else{
    stepper.setSpeedInMillimetersPerSecond(shootingSpeed);
  }
  stepper.setAccelerationInMillimetersPerSecondPerSecond(1.0);
  calculateStats();
}

void writeSettingsToEEPROM(){
  preferences.putFloat("shootingSpeed",shootingSpeed);
  preferences.putFloat("jogSpeed",jogSpeed);
  preferences.putFloat("deshakeDelay",deshakeDelay);
  preferences.putFloat("shootDelay",shutterDelay);
  preferences.putFloat("overshootDist",overshootDistance);
  preferences.putInt("ledBrightness",ledBrightness);
  preferences.putUInt("init",1); //Indicate that EEPROM contains data for next reboot
}

void readSettingsFromEEPROM(){
  if(preferences.getUInt("init",0) != 0){
    shootingSpeed     = preferences.getFloat("shootingSpeed",shootingSpeed);
    jogSpeed          = preferences.getFloat("jogSpeed",jogSpeed);
    deshakeDelay      = preferences.getFloat("deshakeDelay",deshakeDelay);
    shutterDelay      = preferences.getFloat("shootDelay",shutterDelay);
    overshootDistance = preferences.getFloat("overshootDist",overshootDistance);
    ledBrightness     = preferences.getInt("ledBrightness",ledBrightness);
  }else{
    writeSettingsToEEPROM();
  }
}

void setup(void){

  pinMode(SHUTTER_PIN, OUTPUT); 
  pinMode(STEP_EN_PIN, OUTPUT); 
  pinMode(STEP_MS1_PIN, OUTPUT); 
  pinMode(STEP_MS2_PIN, OUTPUT);  
  digitalWrite(SHUTTER_PIN, LOW); 
  digitalWrite(STEP_EN_PIN, LOW); 
  digitalWrite(STEP_MS1_PIN, LOW); // Note: Modifying MS1 or MS2 will change UART address of TMC  
  digitalWrite(STEP_MS2_PIN, LOW); // Note: Modifying MS1 or MS2 will change UART address of TMC
  
  //Init high power LED
  ledcSetup(LED_CHANNEL,5000,8);  
  ledcAttachPin(LED_PWM_PIN, LED_CHANNEL);
  if(ledState){
    ledcWrite(LED_CHANNEL, ledBrightness);
  }else{
    
    ledcWrite(LED_CHANNEL, 0);
  }
  
  Serial.begin(115200);
  Serial.println("");

  preferences.begin("settings",false);
  readSettingsFromEEPROM();

  // When the OpenMacroRail has never been connected to a WiFi network before, 
  // it will set up its own access point that the user can connect to in order
  // to provide credentials for their own network.
  // These credentials are then stored in EEPROM memory and are therefore retained
  // on the MCU until the EEPROM is manually cleared.
  const char* accessPointName = "OpenMacroRail-AP";
  //if (!wifiManager.startConfigPortal(accessPointName)){ // replacing the next line with this one will force the config portal to start
  if(!wifiManager.autoConnect(accessPointName)){
    Serial.println("Failed to connect to WiFi within timeout period. Restarting");
    ESP.restart();
    delay(1000);
  }
  
  Serial.print("\nWiFi connected to: ");
  Serial.println(WiFi.SSID());

  if (MDNS.begin(localDomainName)) {
    Serial.printf("MDNS Responder Started. Connect to http://%s.local or use the following IP address.\n", localDomainName);
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  updateStepperSettings();

  server.on("/", handleRoot);

  server.on("/jogFwd", [](){
    Serial.println("jogFwd");
    JOGFWDFLAG = true;
    server.send(200, "text/plain", "jogFwd");
  });

  server.on("/jogBwd", [](){
    Serial.println("jogBwd");
    JOGBWDFLAG = true;
    server.send(200, "text/plain", "jogBwd");
  });

  server.on("/jogIncrSlow", [](){
    Serial.println("jogIncrSlow");
    jogIncrement = 0.1;
    updateStepperSettings();
    server.send(200, "text/plain", "jogIncrSlow");
  });

  server.on("/jogIncrMedium", [](){
    Serial.println("jogIncrMedium");
    jogIncrement = 0.5;
    updateStepperSettings();
    server.send(200, "text/plain", "jogIncrMedium");
  });

  server.on("/jogIncrFast", [](){
    Serial.println("jogIncrFast");
    jogIncrement = 1.0;
    updateStepperSettings();
    server.send(200, "text/plain", "jogIncrFast");
  });

  server.on("/setStartPoint", [](){
    Serial.println("setStartPoint");
    setStartPoint();
    server.send(200, "text/plain", "setStartPoint");
  });

  server.on("/setEndPoint", [](){
    Serial.println("setEndPoint");
    setEndPoint();
    server.send(200, "text/plain", "setEndPoint");
  });

  server.on("/testImage", [](){
    Serial.println("testImage");
    toggleCameraTrigger();
    server.send(200, "text/plain", "testImage");
  });
  
  server.on("/start", [](){
    Serial.println("start");
    goToStartPoint();
    current_operating_state = PHOTO_SESSION;
    current_session_state = NEXT_IMAGE;
    time_prev_session_state_change = millis();
    picsTaken = 0;
    updateStepperSettings();
    server.send(200, "text/plain", "start");
  });

  server.on("/abort", [](){
    Serial.println("abort");
    current_operating_state=JOGGING;
    picsTaken = 0;
    updateStepperSettings();
    server.send(200, "text/plain", "abort");
  });

  server.on("/numImagesForm", [](){
    Serial.println("numImagesForm");
    if(server.hasArg("num")){
      numImages = server.arg("num").toInt();
      calculateStats();
    }
    server.send(200, "text/plain", String(numImages));
  });

  server.on("/ledSwitch", [](){
    Serial.println("ledSwitch");
    if(server.hasArg("state")){
      ledState = server.arg("state") == "true";
    }
    if(server.hasArg("brightnessPercent")){
      ledBrightness = (server.arg("brightnessPercent").toFloat() / 100) * MAX_LED_BRIGHTNESS;
      preferences.putInt("ledBrightness",ledBrightness);
    }
    calculateStats();
    if(ledState){
      ledcWrite(LED_CHANNEL,ledBrightness);
    }else{
      ledcWrite(LED_CHANNEL,0);
    }
    server.send(200, "text/plain", "ledSwitch: "+String(ledState)+", brightness: "+String(ledBrightness));
  });

  server.on("/shootingSpeedForm", [](){
    Serial.println("shootingSpeedForm");
    if(server.hasArg("num")){
      shootingSpeed = server.arg("num").toFloat();
      updateStepperSettings();
      preferences.putFloat("shootingSpeed",shootingSpeed);
    }
    server.send(200, "text/plain", String(shootingSpeed));
  });

  server.on("/jogSpeedForm", [](){
    Serial.println("jogSpeedForm");
    if(server.hasArg("num")){
      jogSpeed = server.arg("num").toFloat();
      updateStepperSettings();
      preferences.putFloat("jogSpeed",jogSpeed);
    }
    server.send(200, "text/plain", String(jogSpeed));
  });

  server.on("/deshakeDelayForm", [](){
    Serial.println("deshakeDelayForm");
    if(server.hasArg("num")){
      deshakeDelay = server.arg("num").toFloat();
      calculateStats();
      preferences.putFloat("deshakeDelay",deshakeDelay);
    }
    server.send(200, "text/plain", String(deshakeDelay));
  });

  server.on("/shootDelayForm", [](){
    Serial.println("shootDelayForm");
    if(server.hasArg("num")){
      shutterDelay = server.arg("num").toFloat();
      calculateStats();
      preferences.putFloat("shootDelay",shutterDelay);
    }
    server.send(200, "text/plain", String(shutterDelay));
  });

  server.on("/overshootDistanceForm", [](){
    Serial.println("overshootDistanceForm");
    if(server.hasArg("num")){
      overshootDistance = server.arg("num").toFloat();
      calculateStats();
      preferences.putFloat("overshootDist",overshootDistance);
    }
    server.send(200, "text/plain", String(overshootDistance));
  });
  
  server.on("/refreshStats", [](){
    calculateStats();
    String response = "startPoint:"+String(startPoint,3)+","+"endPoint:"+String(endPoint,3)+","+"currentPos:"+String(currentPos,3)+","+"distance:"+String(distance)+","+"increment:"+String(increment,4)+","+"remainingPictures:"+String(remainingPictures)+","+"remainingShootingTime:"+String(remainingShootingTime)+","+"totalShootingTime:"+String(totalShootingTime)+",";
    server.send(200, "text/plain", response);
  });
  
  server.on("/refreshForms", [](){
    String response = "jogIncrement:"+String(jogIncrement,1)+","+"numImages:"+String(numImages)+","+"shootingSpeed:"+String(shootingSpeed,1)+","+"jogSpeed:"+String(jogSpeed,1)+","+"deshakeDelay:"+String(deshakeDelay,1)+","+"shootDelay:"+String(shutterDelay,1)+","+"overshootDistance:"+String(overshootDistance,1)+","+"ledState:"+String(ledState)+","+"ledBrightness:"+String((uint)(((float)ledBrightness/(float)MAX_LED_BRIGHTNESS)*100))+",";
    server.send(200, "text/plain", response);
  });
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop(void){
  server.handleClient();
  long time_now = millis();

  if(current_operating_state==PHOTO_SESSION){
    if((picsTaken<numImages)&&((increment > 0 && currentPos <= startPoint+distance+(increment/2)) || (increment < 0 && currentPos >= startPoint+distance+(increment/2) ))) // +(increment/2) is used as a tolerance for floating point rounding-errors
    {
      if(current_session_state == NEXT_IMAGE){
        Serial.print("Current position: ");
        Serial.print(currentPos);
        Serial.println(" mm");
        time_prev_session_state_change = time_now;
        timeout_period = deshakeDelay*1000;
        current_session_state = WAIT_DESHAKE;
      }else if(time_now-time_prev_session_state_change > timeout_period){ //Note: All other states are wait-states, so they share a common time check.
        // The wait is over
        if(current_session_state == WAIT_DESHAKE){
            activateCameraTrigger();
            time_prev_session_state_change = time_now;
            timeout_period = triggerDelay*1000; //Keep trigger-pin low for a little while to ensure that the signal is registered by the camera.
            current_session_state = WAIT_TRIGGER;
        }else if(current_session_state == WAIT_TRIGGER){
            deactivateCameraTrigger();
            time_prev_session_state_change = time_now;
            timeout_period = shutterDelay*1000; //Wait for shutterDelay to ensure that camera shutter is no longer open before next move
            current_session_state = WAIT_SHUTTER;
        }else if(current_session_state == WAIT_SHUTTER){
          if(picsTaken<numImages){
            //increment position
            stepper.moveRelativeInMillimeters(increment * invert_direction_multiplier);
            currentPos += increment;

            time_prev_session_state_change = time_now;
          }
          current_session_state = NEXT_IMAGE;
        }
      }
    }else{
      current_operating_state = JOGGING;
      current_session_state = NEXT_IMAGE;
      picsTaken = 0;
      updateStepperSettings();
    }
  }else if(current_operating_state==JOGGING){
    if(JOGFWDFLAG){
      jog_fwd();
    }else if(JOGBWDFLAG){
      jog_bwd();
    }
  }
}
