
float panDegreesToSteps(float angle) {
  return pan_steps_per_degree * angle;
}

float tiltDegreesToSteps(float angle) {
  return tilt_steps_per_degree * angle;
}

/*void panHome(){
    while (digitalRead(PIN_PAN_HALL) == 0) { //If already on a Hall sensor move off
      target_position[0] = target_position[0] + panDegreesToSteps(!digitalRead(PIN_PAN_HALL));//increment by 1 degree
      if (target_position[0] > panDegreesToSteps(360)) { //If both axis have done more than a full rotation there must be an issue...
        return false;
      }
      panStepper.moveTo(target_position[0]);
      panStepper.runSpeedToPosition();
    }
    panStepper.setCurrentPosition(0);//set step count to 0
}
*/
/*void tiltHome(){
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
*/
/*bool findHome(void) {
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
*/
float panStepsToDegrees(long steps) {
  return steps / pan_steps_per_degree;
}

float panStepsToDegrees(float steps) {
  return steps / pan_steps_per_degree;
}

float tiltStepsToDegrees(long steps) {
  return steps / tilt_steps_per_degree;
}

float tiltStepsToDegrees(float steps) {
  return steps / tilt_steps_per_degree;
}
