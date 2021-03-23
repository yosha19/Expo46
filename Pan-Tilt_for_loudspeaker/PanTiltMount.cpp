#include "PanTiltMount.h"
#include "Iibrary.h" //A library I created for Arduino that contains some simple functions I commonly use. Library available at: https://github.com/isaac879/Iibrary
#include <AccelStepper.h> //Library to control the stepper motors http://www.airspayce.com/mikem/arduino/AccelStepper/index.html
#include <MultiStepper.h> //Library to control multiple coordinated stepper motors http://www.airspayce.com/mikem/arduino/AccelStepper/classMultiStepper.html#details
#include <EEPROM.h> //To be able to save values when powered off
/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Global scope
AccelStepper stepper_pan = AccelStepper(1, PIN_STEP_PAN, PIN_DIRECTION_PAN);
AccelStepper stepper_tilt = AccelStepper(1, PIN_STEP_TILT, PIN_DIRECTION_TILT);

MultiStepper multi_stepper;

KeyframeElement keyframe_array[KEYFRAME_ARRAY_LENGTH];

int keyframe_elements = 0;
int current_keyframe_index = -1;
char stringText[MAX_STRING_LENGTH + 1];
float pan_steps_per_degree = (200.0 * SIXTEENTH_STEP * PAN_GEAR_RATIO) / 360.0; //Stepper motor has 200 steps per 360 degrees
float tilt_steps_per_degree = (200.0 * SIXTEENTH_STEP * TILT_GEAR_RATIO) / 360.0; //Stepper motor has 200 steps per 360 degrees

int step_mode = SIXTEENTH_STEP;
bool enable_state = true; //Stepper motor driver enable state
float hall_pan_offset_degrees = 0; //Offset to make the pan axis home position centred. This is required because the Hall sensor triggers before being centred on the magnet.
float hall_tilt_offset_degrees = 0; //Offset to make the tilt axis home position centred. This is required because the Hall sensor triggers before being centred on the magnet.
byte invert_pan = 0; //Variables to invert the direction of the axis. Note: These value gets set from the saved EEPROM value on startup.
byte invert_tilt = 0;
byte homing_mode = 0; //Note: Gets set from the saved EEPROM value on startup
float pan_max_speed = 18; //degrees/second. Note: Gets set from the saved EEPROM value on startup.
float tilt_max_speed = 10; //degrees/second.

long target_position[2]; //Array to store stepper motor step counts
float degrees_per_measurement = 0.5; //Note: Gets set from the saved EEPROM value on startup.
unsigned long delay_ms_between_measurements = 1000; //Note: Gets set from the saved EEPROM value on startup.
int pan_accel_increment_us = 4000;
int tilt_accel_increment_us = 3000;
byte acceleration_enable_state = 0;
FloatCoordinate intercept;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/
void initPanTilt(void) {
  Serial.begin(BAUD_RATE);
  pinMode(PIN_MS1, OUTPUT);
  pinMode(PIN_MS2, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_DIRECTION_PAN, OUTPUT);
  pinMode(PIN_STEP_PAN, OUTPUT);
  pinMode(PIN_DIRECTION_TILT, OUTPUT);
  pinMode(PIN_STEP_TILT, OUTPUT);
  pinMode(PIN_PAN_HALL, INPUT_PULLUP);
  pinMode(PIN_TILT_HALL, INPUT_PULLUP);
  setEEPROMVariables();
  setStepMode(step_mode); //steping mode
  stepper_pan.setMaxSpeed(panDegreesToSteps(pan_max_speed));
  stepper_tilt.setMaxSpeed(tiltDegreesToSteps(tilt_max_speed));
  stepper_pan.setAcceleration(5000);
  stepper_tilt.setAcceleration(5000);
  invertPanDirection(invert_pan);
  invertTiltDirection(invert_tilt);
  multi_stepper.addStepper(stepper_pan);
  multi_stepper.addStepper(stepper_tilt);
  digitalWrite(PIN_ENABLE, LOW); //Enable the stepper drivers
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void serialFlush(void) {
  while (Serial.available() > 0) {
    char c = Serial.read();
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void enableSteppers(void) {
  if (enable_state == false) {
    digitalWrite(PIN_ENABLE, LOW); //Enable the stepper drivers
    enable_state = true;
    printi(F("Enabled\n"));
  }
  else {
    digitalWrite(PIN_ENABLE, HIGH); //Disabe the stepper drivers
    enable_state = false;
    printi(F("Disabled\n"));
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void setStepMode(int newMode) { //Step modes for the TMC2208
  float stepRatio = (float)newMode / (float)step_mode; //Ratio between the new step mode and the previously set one.
  if (newMode == HALF_STEP) {
    PORTB |=   B00001000; //MS1 high
    PORTB &= ~(B00000100); //MS2 low
  }
  else if (newMode == QUARTER_STEP) {
    PORTB |=   B00000100; //MS2 high
    PORTB &= ~(B00001000); //MS1 low
  }
  else if (newMode == EIGHTH_STEP) {
    PORTB &= ~(B00001100); //MS1 and MS2 low
  }
  else if (newMode == SIXTEENTH_STEP) {
    PORTB |= B00001100; //MS1 and MS2 high
  }
  else { //If an invalid step mode was entered.
    printi(F("Invalid mode. Enter 2, 4, 8 or 16\n"));
    return;
  }
  //Scale current step to match the new step mode
  stepper_pan.setCurrentPosition(stepper_pan.currentPosition() * stepRatio);
  stepper_tilt.setCurrentPosition(stepper_tilt.currentPosition() * stepRatio);

  pan_steps_per_degree = (200.0 * (float)newMode * PAN_GEAR_RATIO) / 360.0; //Stepper motor has 200 steps per 360 degrees
  tilt_steps_per_degree = (200.0 * (float)newMode * TILT_GEAR_RATIO) / 360.0; //Stepper motor has 200 steps per 360 degrees

  stepper_pan.setMaxSpeed(panDegreesToSteps(pan_max_speed));
  stepper_tilt.setMaxSpeed(tiltDegreesToSteps(tilt_max_speed));
  step_mode = newMode;
  printi(F("Set to "), step_mode, F(" step mode.\n"));
  clearKeyframes();
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void panDegrees(float angle) {
  target_position[0] = panDegreesToSteps(angle);
  if (acceleration_enable_state == 0) {
    multi_stepper.moveTo(target_position);
  }
  else {
    stepper_pan.setCurrentPosition(stepper_pan.currentPosition());
    stepper_pan.runToNewPosition(panDegreesToSteps(angle));
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void tiltDegrees(float angle) {
  target_position[1] = tiltDegreesToSteps(angle);
  if (acceleration_enable_state == 0) {
    multi_stepper.moveTo(target_position);
  }
  else {
    stepper_tilt.setCurrentPosition(stepper_tilt.currentPosition());
    stepper_tilt.runToNewPosition(tiltDegreesToSteps(angle));
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

float panDegreesToSteps(float angle) {
  return pan_steps_per_degree * angle;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

float tiltDegreesToSteps(float angle) {
  return tilt_steps_per_degree * angle;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void printKeyframeElements(void) {
  printi(F("Keyframe index: "), current_keyframe_index, F("\n"));
  for (int row = 0; row < keyframe_elements; row++) {
    printi(F(""), row, F("\t|"));
    printi(F(" Pan: "), panStepsToDegrees(keyframe_array[row].panStepCount), 3, F("º\t"));
    printi(F("Tilt: "), tiltStepsToDegrees(keyframe_array[row].tiltStepCount), 3, F("º\t"));
    printi(F("Pan Speed: "), panStepsToDegrees(keyframe_array[row].panSpeed), 3, F(" º/s\t"));
    printi(F("Tilt Speed: "), tiltStepsToDegrees(keyframe_array[row].tiltSpeed), 3, F(" º/s\t"));
    printi(F("Delay: "), keyframe_array[row].msDelay, F("ms |\n"));
  }
  printi(F("\n"));
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void debugReport(void) {
  //    printi(F("Status\n"));
  printi(F("Status\nEnable state: "), enable_state);
  //    printi(F("Step Mode: "), step_mode);
  printi(F("Pan angle: "), panStepsToDegrees(stepper_pan.currentPosition()), 3, F("º\n"));
  printi(F("Tilt angle: "), tiltStepsToDegrees(stepper_tilt.currentPosition()), 3, F("º\n"));
  printi(F("Pan max steps/s: "), stepper_pan.maxSpeed());
  printi(F("Tilt max steps/s: "), stepper_tilt.maxSpeed());
  printi(F("Pan max speed: "), panStepsToDegrees(stepper_pan.maxSpeed()), 3, F("º/s\n"));
  printi(F("Tilt max speed: "), tiltStepsToDegrees(stepper_tilt.maxSpeed()), 3, F("º/s\n"));
  //    printi(F("Homing mode: "), homing_mode);
  printi(F("Angle between measurements: "), degrees_per_measurement, 3, F("º\n"));
  printi(F("Panoramiclapse delay between measurements: "), delay_ms_between_measurements, F("ms\n"));
  printEEPROM();
  printKeyframeElements();
  //    printi(F("\n"));
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

int setTargetPositions(float panDeg, float tiltDeg) {
  target_position[0] = panDegreesToSteps(panDeg);
  target_position[1] = tiltDegreesToSteps(tiltDeg);
  multi_stepper.moveTo(target_position);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

bool findHome(void) {
  bool panHomeFlag = false;
  bool tiltHomeFlag = false;
  int panHomingDir = -1;
  int tiltHomingDir = -1;

  target_position[0] = stepper_pan.currentPosition();
  target_position[1] = stepper_tilt.currentPosition();

  if (homing_mode == 0) { //No homing
    return false;
  }

  if (homing_mode == 1 || homing_mode == 3) { //Home pan
    while (digitalRead(PIN_PAN_HALL) == 0) { //If already on a Hall sensor move off
      target_position[0] = target_position[0] + panDegreesToSteps(!digitalRead(PIN_PAN_HALL));//increment by 1 degree
      if (target_position[0] > panDegreesToSteps(360)) { //If both axis have done more than a full rotation there must be an issue...
        return false;
      }
      stepper_pan.moveTo(target_position[0]);
      stepper_pan.runSpeedToPosition();
    }
    stepper_pan.setCurrentPosition(0);//set step count to 0
  }

  if (homing_mode == 2 || homing_mode == 3) { //Home tilt
    while (digitalRead(PIN_TILT_HALL) == 0) { //If already on a Hall sensor move off
      target_position[1] = target_position[1] + tiltDegreesToSteps(!digitalRead(PIN_TILT_HALL));//increment by 1 degree
      if (target_position[1] > tiltDegreesToSteps(360)) { //If both axis have done more than a full rotation there must be an issue...
        return false;
      }
      stepper_tilt.moveTo(target_position[1]);
      stepper_tilt.runSpeedToPosition();
    }
    stepper_tilt.setCurrentPosition(0);//set step count to 0
  }

  setTargetPositions(-45, -45);
  while (multi_stepper.run()) {
    if (digitalRead(PIN_PAN_HALL) == 0) {
      stepper_pan.setCurrentPosition(0);//set step count to 0
      setTargetPositions(0, -45 * !tiltHomeFlag);
      panHomeFlag = true;
      panHomingDir = 1;
    }
    if (digitalRead(PIN_TILT_HALL) == 0) {
      stepper_tilt.setCurrentPosition(0);
      setTargetPositions(-45 * !panHomeFlag, 0);
      tiltHomeFlag = true;
      tiltHomingDir = 1;
    }
  }

  setTargetPositions(360 * !panHomeFlag, 360 * !tiltHomeFlag);//full rotation on both axis so it must pass the home position
  while (multi_stepper.run()) {
    if (digitalRead(PIN_PAN_HALL) == 0) {
      stepper_pan.setCurrentPosition(0);//set step count to 0
      setTargetPositions(0, 360 * !tiltHomeFlag);
      panHomeFlag = true;
    }
    if (digitalRead(PIN_TILT_HALL)  == 0) {
      stepper_tilt.setCurrentPosition(0);
      setTargetPositions(360 * !panHomeFlag, 0);
      tiltHomeFlag = true;
    }
  }

  if (panHomeFlag && tiltHomeFlag) {
    setTargetPositions(hall_pan_offset_degrees * panHomingDir, hall_tilt_offset_degrees * tiltHomingDir);
    multi_stepper.runSpeedToPosition();
    stepper_pan.setCurrentPosition(0);//set step count to 0
    stepper_tilt.setCurrentPosition(0);//set step count to 0
    setTargetPositions(0, 0);
    if (homing_mode == 3) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return false;
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

float panStepsToDegrees(long steps) {
  return steps / pan_steps_per_degree;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

float panStepsToDegrees(float steps) {
  return steps / pan_steps_per_degree;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

float tiltStepsToDegrees(long steps) {
  return steps / tilt_steps_per_degree;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

float tiltStepsToDegrees(float steps) {
  return steps / tilt_steps_per_degree;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

int addPosition(void) {
  if (keyframe_elements >= 0 && keyframe_elements < KEYFRAME_ARRAY_LENGTH) {
    keyframe_array[keyframe_elements].panStepCount = stepper_pan.currentPosition();
    keyframe_array[keyframe_elements].tiltStepCount = stepper_tilt.currentPosition();
    keyframe_array[keyframe_elements].panSpeed = stepper_pan.maxSpeed();
    keyframe_array[keyframe_elements].tiltSpeed = stepper_tilt.maxSpeed();
    keyframe_array[keyframe_elements].msDelay = 0;
    current_keyframe_index = keyframe_elements;
    keyframe_elements++;//increment the index
    printi(F("Added at index: "), current_keyframe_index);
    return 0;
  }
  else {
    printi(F("Max number of keyframes reached\n"));
  }
  return -1;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void clearKeyframes(void) {
  keyframe_elements = 0;
  current_keyframe_index = -1;
  printi(F("Keyframes cleared\n"));
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void moveToIndex(int index) {
  if (index < keyframe_elements && index >= 0) {
    target_position[0] = keyframe_array[index].panStepCount;
    target_position[1] = keyframe_array[index].tiltStepCount;
    stepper_pan.setMaxSpeed(keyframe_array[index].panSpeed);
    stepper_tilt.setMaxSpeed(keyframe_array[index].tiltSpeed);

    if (acceleration_enable_state == 0) { //If accelerations are not enabled just move directly to the target position.
      multi_stepper.moveTo(target_position); //Sets new target positions
      multi_stepper.runSpeedToPosition(); //Moves and blocks until complete
      delay(keyframe_array[index].msDelay);
      current_keyframe_index = index;
      return;
    }

    float panInitialSpeed = stepper_pan.speed();
    float tiltInitialSpeed = stepper_tilt.speed();

    if (index >= 1) {
      if (keyframe_array[index - 1].msDelay != 0) {
        panInitialSpeed = 0;
        tiltInitialSpeed = 0;
      }
    }

    multi_stepper.moveTo(target_position); //Sets new target positions //sets speeds

    float panDeltaSpeed = stepper_pan.speed() - panInitialSpeed;
    float tiltDeltaSpeed = stepper_tilt.speed() - tiltInitialSpeed;

    float panAccel = stepper_pan.speed() / (pan_accel_increment_us * 0.0001); //Equation is arbitrary and was deterined through empirical testing. The acceleration value does NOT correspond to mm/s/s
    float tiltAccel = stepper_tilt.speed() / (tilt_accel_increment_us * 0.0001);

    long panDist = 0;
    long tiltDist = 0;

    if (panAccel != 0) {
      panDist = pow(stepper_pan.speed(), 2) / (5 * panAccel); //Equation is arbitrary and was deterined through empirical testing.
    }
    if (tiltAccel != 0) {
      tiltDist = pow(stepper_tilt.speed(), 2) / (5 * tiltAccel); //Equation is arbitrary and was deterined through empirical testing.
    }

    if (index + 1 < keyframe_elements) { //makes sure there is a valid next keyframe
      if (keyframe_array[index].msDelay == 0) {
        long panStepDiff = keyframe_array[index + 1].panStepCount - keyframe_array[index].panStepCount; //Change in position from current target position to the next.
        long tiltStepDiff = keyframe_array[index + 1].tiltStepCount - keyframe_array[index].tiltStepCount;
        if ((panStepDiff == 0 && stepper_pan.speed() != 0) || (panStepDiff > 0 && stepper_pan.speed() < 0) || (panStepDiff < 0 && stepper_pan.speed() > 0)) { //if stopping or changing direction
          target_position[0] = keyframe_array[index].panStepCount - panDist; //Set the target position slightly before the actual target to allow for the distance traveled while decelerating.
        }
        if ((tiltStepDiff == 0 && stepper_tilt.speed() != 0) || (tiltStepDiff > 0 && stepper_tilt.speed() < 0) || (tiltStepDiff < 0 && stepper_tilt.speed() > 0)) { //if stopping or changing direction
          target_position[1] = keyframe_array[index].tiltStepCount - tiltDist;
        }

      }
    }

    if (index > 0) {
      long panStepDiffPrev = keyframe_array[index].panStepCount - keyframe_array[index - 1].panStepCount; //Change in position from the privious target to the current target position.
      long tiltStepDiffPrev = keyframe_array[index].tiltStepCount - keyframe_array[index - 1].tiltStepCount;
      if (panStepDiffPrev == 0 && panDeltaSpeed == 0) { //Movement stopping
        panDeltaSpeed = -(2 * stepper_pan.speed()); //Making it negative ramps the speed down in the acceleration portion of the movement. The multiplication factor is arbitrary and was deterined through empirical testing.
      }
      if (tiltStepDiffPrev == 0 && tiltDeltaSpeed == 0) { //Movement stopping
        tiltDeltaSpeed = -(2 * stepper_tilt.speed());
      }

    }

    multi_stepper.moveTo(target_position); //Sets new target positions and calculates new speeds.

    if (stepper_pan.currentPosition() != target_position[0] || stepper_tilt.currentPosition() != target_position[1]) { //Prevents issues caused when the motor target positions and speeds not being updated becuase they have not changed.
      //Impliments the acceleration/deceleration. This implimentation feels pretty bad and should probably be updated but it works well enough so I'm not going to...
      float panInc = 0;
      float tiltInc = 0;
      unsigned long pan_last_us = 0;
      unsigned long tilt_last_us = 0;

      while (((panInc < 1) || (tiltInc < 1)) && multi_stepper.run()) {
        unsigned long usTime = micros();

        if (usTime - pan_accel_increment_us >= pan_last_us) {
          panInc = (panInc < 1) ? (panInc + 0.01) : 1;
          pan_last_us = micros();
          stepper_pan.setSpeed(panInitialSpeed + (panDeltaSpeed * panInc));
        }

        if (usTime - tilt_accel_increment_us >= tilt_last_us) {
          tiltInc = (tiltInc < 1) ? (tiltInc + 0.01) : 1;
          tilt_last_us = micros();
          stepper_tilt.setSpeed(tiltInitialSpeed + (tiltDeltaSpeed * tiltInc));
        }

      }

      multi_stepper.moveTo(target_position); //Sets all speeds to reach the target
      multi_stepper.runSpeedToPosition(); //Moves and blocks until complete
    }
    delay(keyframe_array[index].msDelay);
    current_keyframe_index = index;
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void executeMoves(int repeat) {
  stepper_pan.setSpeed(0);
  stepper_tilt.setSpeed(0);
  for (int i = 0; i < repeat; i++) {
    for (int row = 0; row < keyframe_elements; row++) {
      moveToIndex(row);
    }
    
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void gotoFirstKeyframe(void) {
  moveToIndex(0);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void gotoLastKeyframe(void) {
  moveToIndex(keyframe_elements - 1);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void editKeyframe(void) {
  keyframe_array[current_keyframe_index].panStepCount = stepper_pan.currentPosition();
  keyframe_array[current_keyframe_index].tiltStepCount = stepper_tilt.currentPosition();
  keyframe_array[current_keyframe_index].panSpeed = stepper_pan.maxSpeed();
  keyframe_array[current_keyframe_index].tiltSpeed = stepper_tilt.maxSpeed();

  printi(F("Edited index: "), current_keyframe_index);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void editDelay(unsigned int ms) {
  keyframe_array[current_keyframe_index].msDelay = ms;
  printi(ms, F(""));
  printi(F("ms delay added at index: "), current_keyframe_index);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void addDelay(unsigned int ms) {
  addPosition();
  keyframe_array[current_keyframe_index].msDelay = ms;
  printi(ms, F(""));
  printi(F("ms delay added at index: "), current_keyframe_index);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void invertPanDirection(bool invert) {
  printi(F("Pan inversion: "), invert);
  invert_pan = invert;
  stepper_pan.setPinsInverted(invert, false, false);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void invertTiltDirection(bool invert) {
  printi(F("Tilt inversion: "), invert);
  invert_tilt = invert;
  stepper_tilt.setPinsInverted(invert, false, false);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void saveEEPROM(void) {
  EEPROM.put(EEPROM_ADDRESS_HOMING_MODE, homing_mode);
  EEPROM.put(EEPROM_ADDRESS_MODE, step_mode);
  EEPROM.put(EEPROM_ADDRESS_PAN_MAX_SPEED, pan_max_speed);
  EEPROM.put(EEPROM_ADDRESS_TILT_MAX_SPEED, tilt_max_speed);
  EEPROM.put(EEPROM_ADDRESS_HALL_PAN_OFFSET, hall_pan_offset_degrees);
  EEPROM.put(EEPROM_ADDRESS_HALL_TILT_OFFSET, hall_tilt_offset_degrees);
  EEPROM.put(EEPROM_ADDRESS_INVERT_PAN, invert_pan);
  EEPROM.put(EEPROM_ADDRESS_INVERT_TILT, invert_tilt);
  EEPROM.put(EEPROM_ADDRESS_DEGREES_PER_MEASUREMENT, degrees_per_measurement);
  EEPROM.put(EEPROM_ADDRESS_ACCELERATION_ENABLE, acceleration_enable_state);
  EEPROM.put(EEPROM_ADDRESS_PAN_ACCEL_INCREMENT_DELAY, pan_accel_increment_us);
  EEPROM.put(EEPROM_ADDRESS_TILT_ACCEL_INCREMENT_DELAY, tilt_accel_increment_us);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void printEEPROM(void) {
  int itemp;
  float ftemp;
  long ltemp;
  //    printi(F("EEPROM:\n"));
  EEPROM.get(EEPROM_ADDRESS_MODE, itemp);
  printi(F("EEPROM:\nStep mode: "), itemp, F("\n"));
  EEPROM.get(EEPROM_ADDRESS_PAN_MAX_SPEED, ftemp);
  printi(F("Pan max: "), ftemp, 3, F("º/s\n"));
  EEPROM.get(EEPROM_ADDRESS_TILT_MAX_SPEED, ftemp);
  printi(F("Tilt max: "), ftemp, 3, F("º/s\n"));
  EEPROM.get(EEPROM_ADDRESS_HALL_PAN_OFFSET, ftemp);
  printi(F("Pan offset: "), ftemp, 3, F("º\n"));
  EEPROM.get(EEPROM_ADDRESS_HALL_TILT_OFFSET, ftemp);
  printi(F("Tilt offset: "), ftemp, 3, F("º\n"));
  EEPROM.get(EEPROM_ADDRESS_DEGREES_PER_MEASUREMENT, ftemp);
  printi(F("Angle between measurements: "), ftemp, 3, F(" º\n"));
  printi(F("Delay between measurements: "), ltemp, F("ms\n"));
  printi(F("Pan invert: "), EEPROM.read(EEPROM_ADDRESS_INVERT_PAN));
  printi(F("Tilt invert: "), EEPROM.read(EEPROM_ADDRESS_INVERT_TILT));
  printi(F("Homing mode: "), EEPROM.read(EEPROM_ADDRESS_HOMING_MODE));
  printi(F("Accel enable: "), EEPROM.read(EEPROM_ADDRESS_ACCELERATION_ENABLE));
  EEPROM.get(EEPROM_ADDRESS_PAN_ACCEL_INCREMENT_DELAY, itemp);
  printi(F("Pan accel delay: "), itemp, F("us\n"));
  EEPROM.get(EEPROM_ADDRESS_TILT_ACCEL_INCREMENT_DELAY, itemp);
  printi(F("Tilt accel delay: "), itemp, F("us\n"));
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void setEEPROMVariables(void) {
  EEPROM.get(EEPROM_ADDRESS_MODE, step_mode);
  EEPROM.get(EEPROM_ADDRESS_PAN_MAX_SPEED, pan_max_speed);
  EEPROM.get(EEPROM_ADDRESS_TILT_MAX_SPEED, tilt_max_speed);
  EEPROM.get(EEPROM_ADDRESS_HALL_PAN_OFFSET, hall_pan_offset_degrees);
  EEPROM.get(EEPROM_ADDRESS_HALL_TILT_OFFSET, hall_tilt_offset_degrees);
  EEPROM.get(EEPROM_ADDRESS_DEGREES_PER_MEASUREMENT, degrees_per_measurement);
  EEPROM.get(EEPROM_ADDRESS_PAN_ACCEL_INCREMENT_DELAY, pan_accel_increment_us);
  EEPROM.get(EEPROM_ADDRESS_TILT_ACCEL_INCREMENT_DELAY, tilt_accel_increment_us);
  invert_pan = EEPROM.read(EEPROM_ADDRESS_INVERT_PAN);
  invert_tilt = EEPROM.read(EEPROM_ADDRESS_INVERT_TILT);
  homing_mode = EEPROM.read(EEPROM_ADDRESS_HOMING_MODE);
  acceleration_enable_state = EEPROM.read(EEPROM_ADDRESS_ACCELERATION_ENABLE);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void setHoming(byte homingType) {
  if (homingType >= 0 && homingType <= 4) {
    homing_mode = homingType;
    printi(F("Homing set to mode "), homingType, "\n");
  }
  else {
    printi(F("Invalid mode\n"));
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void toggleAcceleration(void) {
  if (acceleration_enable_state == 0) {
    acceleration_enable_state = 1;
    printi(F("Accel enabled.\n"));
  }
  else {
    acceleration_enable_state = 0;
    printi(F("Accel disabled.\n"));
  }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

void scaleKeyframeSpeed(float scaleFactor) {
  if (scaleFactor <= 0) { //Make sure a valid speed factor was entered
    printi(F("Invalid factor\n"));
    return;
  }

  for (int row = 0; row < keyframe_elements; row++) {
    keyframe_array[row].panSpeed *= scaleFactor;
    keyframe_array[row].tiltSpeed *= scaleFactor;
  }
  printi(F("Keyframe speed scaled by "), scaleFactor, 3, F("\n"));
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/
void serialData(void) {
  char instruction = Serial.read();
  if (instruction == INSTRUCTION_BYTES_PAN_TILT_SPEED) {
    int count = 0;
    while (Serial.available() < 6) { //Wait for 6 bytes to be available. Breaks after ~20ms if bytes are not received.
      delayMicroseconds(200);
      count++;
      if (count > 100) {
        serialFlush();//Clear the serial buffer
        break;
      }
    }
    int panStepSpeed = (Serial.read() << 8) + Serial.read();
    int tiltStepSpeed = (Serial.read() << 8) + Serial.read();

    stepper_pan.setSpeed(panStepSpeed);
    stepper_tilt.setSpeed(tiltStepSpeed);
    stepper_pan.runSpeed();
    stepper_tilt.runSpeed();
  }

  delay(2); //wait to make sure all data in the serial message has arived
  memset(&stringText[0], 0, sizeof(stringText)); //clear the array
  while (Serial.available()) { //set elemetns of stringText to the serial values sent
    char digit = Serial.read(); //read in a char
    strncat(stringText, &digit, 1); //add digit to the end of the array
  }
  serialFlush();//Clear any excess data in the serial buffer
  int serialCommandValueInt = atoi(stringText); //converts stringText to an int
  float serialCommandValueFloat = atof(stringText); //converts stringText to a float
  if (instruction == '+') { //The Bluetooth module sends a message starting with "+CONNECTING" which should be discarded.
    delay(100); //wait to make sure all data in the serial message has arived
    serialFlush();//Clear any excess data in the serial buffer
    return;
  }
  switch (instruction) {
    case INSTRUCTION_SCALE_SPEED: {
        scaleKeyframeSpeed(serialCommandValueFloat);
      }
      break;
    case INSTRUCTION_PAN_ACCEL_INCREMENT_DELAY: {
        pan_accel_increment_us = (serialCommandValueInt >= 0) ? serialCommandValueInt : 0;
        printi(F("Pan accel delay: "), pan_accel_increment_us, F("us\n"));
      }
      break;
    case INSTRUCTION_TILT_ACCEL_INCREMENT_DELAY: {
        tilt_accel_increment_us = (serialCommandValueInt >= 0) ? serialCommandValueInt : 0;
        printi(F("Tilt accel delay: "), tilt_accel_increment_us, F("us\n"));
      }
      break;
    case INSTRUCTION_ACCEL_ENABLE: {
        toggleAcceleration();
      }
      break;
    case INSTRUCTION_DELAY_BETWEEN_MEASUREMENTS: {
        delay_ms_between_measurements = serialCommandValueFloat;
        printi(F("Delay between measurements: "), delay_ms_between_measurements, F("ms\n"));
      }
      break;
    case INSTRUCTION_ANGLE_BETWEEN_MEASUREMENTS: {
        degrees_per_measurement = serialCommandValueFloat;
        printi(F("Degs per measurement: "), degrees_per_measurement, 3, F("º\n"));
      }
      break;
    case INSTRUCTION_AUTO_HOME: {
        printi(F("Homing\n"));
        if (findHome()) {
          printi(F("Complete\n"));
        }
        else {
          stepper_pan.setCurrentPosition(0);
          stepper_tilt.setCurrentPosition(0);
          setTargetPositions(0, 0);
          printi(F("Error homing\n"));
        }
      }
      break;
    case INSTRUCTION_SET_HOMING: {
        setHoming(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_SET_PAN_HALL_OFFSET: {
        hall_pan_offset_degrees = serialCommandValueFloat;
        printi(F("Pan offset: "), hall_pan_offset_degrees, 3, F("º\n"));
      }
      break;
    case INSTRUCTION_SET_TILT_HALL_OFFSET: {
        hall_tilt_offset_degrees = serialCommandValueFloat;
        printi(F("Tilt offset: "), hall_tilt_offset_degrees, 3, F("º\n"));
      }
      break;
    case INSTRUCTION_INVERT_TILT: {
        invertTiltDirection(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_INVERT_PAN: {
        invertPanDirection(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_SAVE_TO_EEPROM: {
        saveEEPROM();
        printi(F("Saved to EEPROM\n"));
      }
      break;
    case INSTRUCTION_ADD_POSITION: {
        addPosition();
      }
      break;
    case INSTRUCTION_STEP_FORWARD: {
        moveToIndex(current_keyframe_index + 1);
        printi(F("Index: "), current_keyframe_index, F("\n"));
      }
      break;
    case INSTRUCTION_STEP_BACKWARD: {
        moveToIndex(current_keyframe_index - 1);
        printi(F("Index: "), current_keyframe_index, F("\n"));
      }
      break;
    case INSTRUCTION_JUMP_TO_START: {
        gotoFirstKeyframe();
        printi(F("Index: "), current_keyframe_index, F("\n"));
      }
      break;
    case INSTRUCTION_JUMP_TO_END: {
        gotoLastKeyframe();
        printi(F("Index: "), current_keyframe_index, F("\n"));
      }
      break;
    case INSTRUCTION_EDIT_ARRAY: {
        editKeyframe();
      }
      break;
    case INSTRUCTION_ADD_DELAY: {
        addDelay(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_EDIT_DELAY: {
        editDelay(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_CLEAR_ARRAY: {
        clearKeyframes();
      }
      break;
    case INSTRUCTION_EXECUTE_MOVES: {
        executeMoves(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_DEBUG_STATUS: {
        debugReport();
      }
      break;
    case INSTRUCTION_PAN_DEGREES: {
        panDegrees(serialCommandValueFloat);
      }
      break;
    case INSTRUCTION_TILT_DEGREES: {
        tiltDegrees(serialCommandValueFloat);
      }
      break;
    case INSTRUCTION_ENABLE: {
        enableSteppers();
      }
      break;
    case INSTRUCTION_STEP_MODE: {
        setStepMode(serialCommandValueInt);
      }
      break;
    case INSTRUCTION_SET_PAN_SPEED: {
        printi(F("Max pan speed: "), serialCommandValueFloat, 1, "º/s.\n");
        pan_max_speed = serialCommandValueFloat;
        stepper_pan.setMaxSpeed(panDegreesToSteps(pan_max_speed));
      }
      break;
    case INSTRUCTION_SET_TILT_SPEED: {
        printi(F("Max tilt speed: "), serialCommandValueFloat, 1, "º/s.\n");
        tilt_max_speed = serialCommandValueFloat;
        stepper_tilt.setMaxSpeed(tiltDegreesToSteps(tilt_max_speed));
      }
      break;
  }
}
