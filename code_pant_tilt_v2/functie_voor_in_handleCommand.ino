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
float panDegreesToSteps(float angle) {
  return pan_steps_per_degree * angle;
}

float tiltDegreesToSteps(float angle) {
  return tilt_steps_per_degree * angle;
}

void panHome() {
  long target = panDegreesToSteps(360);
  long p = panDegreesToSteps(12.0);
  panStepper.moveTo(target);
  while ( digitalRead(PIN_PAN_HALL) != 0){
    Serial.println("pah runnen");
    panStepper.run();
  }
  Serial.println(PIN_PAN_HALL);
  Serial.println("pah stop");
  panStepper.setCurrentPosition(0);//set step count to 0
  panStepper.moveTo(0);
  panHomeFlag = true;
  panHomingDir = 1;
}

void homeStepper(AccelStepper& stepper, uint8_t hallPin, float offset, long maxSteps) {
  stepper.move(maxSteps);

  // Wait until we don't see the homing magnet (probably immediately)
  while (digitalRead(hallPin) == 0 && stepper.run());

  // Wait until we see the homing magnet (should take up to one rotation
  while ( digitalRead(hallPin) != 0 && stepper.run());
  long hallDetectionBegin = stepper.currentPosition();

  while (digitalRead(hallPin) == 0 && stepper.run());
  long hallDetectionEnd = stepper.currentPosition();
  long hallDetectionMiddle = (hallDetectionBegin + hallDetectionEnd) / 2;

  Serial.println("tah stop");
//  stepper.runToNewPosition(stepper.currentPosition() + tiltDegreesToSteps(offset));
  stepper.runToNewPosition(hallDetectionMiddle + tiltDegreesToSteps(offset));
//  while (tiltStepper.run());
  stepper.setCurrentPosition(0);//set step count to 0
//  tiltHomeFlag = true;
//  tiltHomingDir = 1;
  
}
void tiltHome() {
  long t = tiltDegreesToSteps(12.0);
  long target = tiltDegreesToSteps(360);
  tiltStepper.moveTo(target);
  while ( digitalRead(PIN_TILT_HALL) != 0 && tiltStepper.run());
  Serial.println("tah stop");
  tiltStepper.runToNewPosition(tiltStepper.currentPosition() + t);
//  while (tiltStepper.run());
  tiltStepper.setCurrentPosition(0);//set step count to 0
  tiltHomeFlag = true;
  tiltHomingDir = 1;
  
}
