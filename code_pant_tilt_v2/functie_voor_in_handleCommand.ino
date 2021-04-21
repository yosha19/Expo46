
float panDegreesToSteps(float angle) {
  return pan_steps_per_degree * angle;
}

float tiltDegreesToSteps(float angle) {
  return tilt_steps_per_degree * angle;
}

/*void panHome(){

      //panStepper.currentPosition();
      float target = panDegreesToSteps(-90);
      panStepper.moveTo(target);
    if (digitalRead(PIN_PAN_HALL) == 0) {
      Serial.println(PIN_PAN_HALL);
      panStepper.setCurrentPosition(0);//set step count to 0
      panStepper.moveTo(0);
      panHomeFlag = true;
      panHomingDir = 1;
    }
  }
*/  
/*void tiltHome(){

    tiltStepper.currentPosition();
    long target = tiltDegreesToSteps(-90);
    tiltStepper.moveTo(target);
    if (digitalRead(PIN_TILT_HALL) == 0) {
      tiltStepper.setCurrentPosition(0);//set step count to 0
      tiltStepper.moveTo(0);
      tiltHomeFlag = true;
      tiltHomingDir = 1;
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
