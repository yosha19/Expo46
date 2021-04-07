
#include <AccelStepper.h>

//Pin definitions when using Arduino Nano pinout
#define BAUD_RATE 9600
#define PIN_PAN_HALL A3
#define PIN_TILT_HALL A4
#define PIN_INPUT_VOLTAGE A5
#define PIN_ENABLE 8 
#define PIN_MS1 11
#define PIN_MS2 10
#define PIN_STEP_TILT 6
#define PIN_DIRECTION_TILT 3
#define PIN_STEP_PAN 7
#define PIN_DIRECTION_PAN 4

#define HALF_STEP 2
#define QUARTER_STEP 4
#define EIGHTH_STEP 8
#define SIXTEENTH_STEP 16

#define PAN_GEAR_RATIO 144/17 //44/17  // teeth
//Used with the belt driven tilt axis.
#define TILT_GEAR_RATIO 64/21 // teeth

AccelStepper panStepper= AccelStepper(1, PIN_STEP_PAN, PIN_DIRECTION_PAN);
AccelStepper tiltStepper= AccelStepper(1, PIN_STEP_TILT, PIN_DIRECTION_TILT);


float pan_steps_per_degree = (200.0 * 8 * PAN_GEAR_RATIO) / 360.0; //Stepper motor has 200 steps per 360 degrees
float tilt_steps_per_degree = (200.0 * 8 * TILT_GEAR_RATIO) / 360.0; //Stepper motor has 200 steps per 360 degrees
float pan_max_speed = 18; //degrees/second. 
float tilt_max_speed = 10; //degrees/second.
//int step_mode = SIXTEENTH_STEP;
float hall_pan_offset_degrees = 0; //Offset to make the pan axis home position centred. This is required because the Hall sensor triggers before being centred on the magnet.
float hall_tilt_offset_degrees = 0; //Offset to make the tilt axis home position centred. This is required because the Hall sensor triggers before being centred on the magnet.

void setup() {
  // put your setup code here, to run once:
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

  panStepper.setAcceleration(5000);
  panStepper.setMaxSpeed(panDegreesToSteps(pan_max_speed));
  tiltStepper.setAcceleration(5000);
  tiltStepper.setMaxSpeed(tiltDegreesToSteps(tilt_max_speed));
  digitalWrite(PIN_ENABLE, LOW); //Enable the stepper drivers
  //digitalWrite(PIN_MS1, HIGH); //Enable the stepper drivers
  //digitalWrite(PIN_MS2, HIGH); //Enable the stepper drivers
  //PORTB |= B00001100;
}

// key=value => value
// Assumes that line does not contain line endings
String testioGetValue(String line) {
  return line.substring(line.indexOf('=') + 1);
}

int testioGetInt(String line) {
  return testioGetValue(line).toInt();
}

long testioGetLong(String line) {
  return atol(testioGetValue(line).c_str());
}

float testioGetFloat(String line) {
  return testioGetValue(line).toFloat();
}

void handleCommand(String command) {
  if (command.startsWith("p=")){
    float target = panDegreesToSteps(testioGetFloat(command));
    panStepper.moveTo(target);
    Serial.print("p=");
    Serial.println(target);
 
  } else if (command.startsWith("tnewpos=")) {
    float target = tiltDegreesToSteps(testioGetFloat(command));
    tiltStepper.moveTo(target);
    Serial.print("tnewpos=");
    Serial.println(target);

  } else if (command == "p_target?") {
    Serial.print("p_target=");
    Serial.println(panStepsToDegrees(panStepper.targetPosition()));

  } else if (command == "t_target?") {
    Serial.print("t_target=");
    Serial.println(tiltStepsToDegrees(tiltStepper.targetPosition()));

    } else if (command.startsWith("ppos=")) {
    float pos = panDegreesToSteps(testioGetFloat(command));
    panStepper.setCurrentPosition(pos);
    Serial.print("z_pos=");
    Serial.println(pos);

    } else if (command.startsWith("tpos=")) {
    float pos = tiltDegreesToSteps(testioGetFloat(command));
    tiltStepper.setCurrentPosition(pos);
    Serial.print("tpos=");
    Serial.println(pos);

  } else if (command == "p_pos?") {
    Serial.print("p_pos=");
    Serial.println(panStepsToDegrees(panStepper.currentPosition()));

  } else if (command == "t_pos?") {
    Serial.print("t_pos=");
    Serial.println(tiltStepsToDegrees(tiltStepper.currentPosition()));

  } else if (command == "pos?") {
    Serial.print("p_pos=");
    Serial.println(panStepsToDegrees(panStepper.currentPosition()));
    Serial.print("t_pos=");
    Serial.println(tiltStepsToDegrees(tiltStepper.currentPosition()));

  } else if (command.startsWith("p_accel=")) {
    float accel = panDegreesToSteps(testioGetFloat(command));
    panStepper.setAcceleration(accel);
    Serial.print("p_accel=");
    Serial.println(accel);

  } else if (command.startsWith("t_accel=")) {
    float accel = tiltDegreesToSteps(testioGetFloat(command));
    tiltStepper.setAcceleration(accel);
    Serial.print("t_accel=");
    Serial.println(accel);

  } else if (command.startsWith("p_max_speed=")) {
    float maxspeed = panDegreesToSteps(testioGetFloat(command));
    panStepper.setMaxSpeed(maxspeed);
    Serial.print("p_max_speed=");
    Serial.println(maxspeed);

  } else if (command.startsWith("t_max_speed=")) {
    float maxspeed = tiltDegreesToSteps(testioGetFloat(command));
    tiltStepper.setMaxSpeed(maxspeed);
    Serial.print("t_max_speed=");
    Serial.println(maxspeed);

  } else if (command == "p_max_speed?") {
    Serial.print("p_max_speed=");
    Serial.println(panStepsToDegrees(panStepper.maxSpeed()));

  } else if (command == "t_max_speed?") {
    Serial.print("t_max_speed=");
    Serial.println(tiltStepsToDegrees(tiltStepper.maxSpeed()));

  } else if (command.startsWith("p_enable=")) {
    if (testioGetInt(command)) {
      digitalWrite(PIN_ENABLE, LOW);
      Serial.println("p_enable=1");
    } else {
      digitalWrite(PIN_ENABLE, HIGH);
      Serial.println("p_enable=0");
    }

  } else if (command == "p_enable?") {
    Serial.print("p_enable=");
    Serial.println(!digitalRead(PIN_ENABLE));
    
  } else if (command == "panHome") {
    // panHome();
    Serial.print("panHome");

  } else if (command == "tilthome") {
    // tiltHome();
    Serial.print("tiltHome");

    

  } else {
    Serial.print("Unknown command: ");
    Serial.println(command);

  }
}

String command;
void loop() {
  
  if (Serial.available()) {
    char c = Serial.read();
    if (c != '\r' && c != '\n') {
      command += c;
    } else {
      handleCommand(command);
      command = "";
    }
  }

  panStepper.run();
  tiltStepper.run();
}
