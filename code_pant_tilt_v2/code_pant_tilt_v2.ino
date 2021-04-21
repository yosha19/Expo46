
#include <AccelStepper.h>

//Pin definitions when using Arduino Nano pinout
#define BAUD_RATE 9600
#define PIN_PAN_HALL A6
#define PIN_TILT_HALL A7
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
#define TILT_GEAR_RATIO 64/21 // teeth

AccelStepper panStepper = AccelStepper(1, PIN_STEP_PAN, PIN_DIRECTION_PAN);
AccelStepper tiltStepper = AccelStepper(1, PIN_STEP_TILT, PIN_DIRECTION_TILT);


float pan_steps_per_degree = (200.0 * 1 * PAN_GEAR_RATIO) / 360.0 / 2; //Stepper motor has 200 steps per 360 degrees, dus 200/360 geeft aantal stappen per graad, deel je dit nog eens door 2 heb je het aantal stappen per halve graad. 
                                                                       //(begin regel hierboven met lezen) Het kleine tandwiel moet sneller draaien dan het grote tandwiel, waarvoor de verhouding van 144/17 geldt. 
                                                                       //Maar andersom gekeken zal het grote tandwiel dus langzamer moeten draaien dus geldt de verhouding van 17/144.
                                                                       //De motor stuurt het kleine tandwiel aan waardoor de verhouding van 144/17 geldt. Ditzelfde principe geldt voor de motor voor tilting.
float tilt_steps_per_degree = (200.0 * 1 * TILT_GEAR_RATIO) / 360.0 / 2; //Stepper motor has 200 steps per 360 degrees
float pan_max_speed = 18; //degrees/second. Deze kan dus aangepast worden naar hoelang de beweging mag duren. De eis was om binnen 4 sec 45 graden te halen, en dat haal je hier wel mee.
float tilt_max_speed = 10; //degrees/second.
bool panHomeFlag = false;
bool tiltHomeFlag = false;
int panHomingDir = -1;
int tiltHomingDir = -1;
int hallState1 = 0;
int hallState2 = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(PIN_MS1, OUTPUT);
  pinMode(PIN_MS2, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_DIRECTION_PAN, OUTPUT);
  pinMode(PIN_STEP_PAN, OUTPUT);
  pinMode(PIN_DIRECTION_TILT, OUTPUT);
  pinMode(PIN_STEP_TILT, OUTPUT);
  pinMode(PIN_PAN_HALL, INPUT);
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
  if (command.startsWith("p_target=")) {
    long target = panDegreesToSteps(testioGetFloat(command));
    panStepper.moveTo(target);
    Serial.print("p_target=");
    Serial.println(target);

  } else if (command == "p_target?") {
    Serial.print("p_target=");
    Serial.println(panStepsToDegrees(panStepper.targetPosition()));

  } else if (command.startsWith("t_target=")) {
    long target = tiltDegreesToSteps(testioGetFloat(command));
    tiltStepper.moveTo(target);
    Serial.print("t_target=");
    Serial.println(target);

  } else if (command == "t_target?") {
    Serial.print("t_target=");
    Serial.println(tiltStepsToDegrees(tiltStepper.targetPosition()));

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

  } else if (command == "p_stop") {
    panStepper.stop();
    Serial.print("p_stop");

  } else if (command == "t_stop") {
    tiltStepper.stop();
    Serial.print("t_stop");

  } else if (command == "stop") {
    panStepper.stop();
    tiltStepper.stop();
    Serial.print("stop");

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
      //panStepper.moveTo(-100);
      hallState1 = digitalRead(PIN_PAN_HALL);
      while (panStepper.run()) {
    if (hallState1 == 0) {
      panStepper.setCurrentPosition(0);//set step count to 0
      //panStepper.moveTo(0);
      panHomeFlag = true;
      panHomingDir = 1;
    }
      Serial.println(hallState1);
      Serial.println("panHome");
      //Serial.println(panHomeFlag);
      }
      
  } else if (command == "panHome?") {
    Serial.print("panHome=");
    Serial.println(panHomeFlag);

  } else if (command == "tilthome") {
    //tiltHome();
    //tiltStepper.moveTo(-100);
      hallState2 = digitalRead(PIN_TILT_HALL);
      while (tiltStepper.run()) {
    if (hallState2 == 0) {
      tiltStepper.setCurrentPosition(0);//set step count to 0
      //panStepper.moveTo(0);
      tiltHomeFlag = true;
      tiltHomingDir = 1;
    }
    Serial.print("tiltHome");
      }
  } else if (command == "tiltHome?") {
    Serial.print("tiltHome=");
    Serial.println(tiltHomeFlag);

  } else if (command == "?") {
    Serial.println("p_target=");
    Serial.println(panStepsToDegrees(panStepper.targetPosition()));
    Serial.println("t_target=");
    Serial.println(tiltStepsToDegrees(tiltStepper.targetPosition()));
    Serial.println("p_pos=");
    Serial.println(panStepsToDegrees(panStepper.currentPosition()));
    Serial.println("t_pos=");
    Serial.println(tiltStepsToDegrees(tiltStepper.currentPosition()));
    Serial.println("p_max_speed=");
    Serial.println(panStepsToDegrees(panStepper.maxSpeed()));
    Serial.println("t_max_speed=");
    Serial.println(tiltStepsToDegrees(tiltStepper.maxSpeed()));
    Serial.println("p_enable=");
    Serial.println(!digitalRead(PIN_ENABLE));
    Serial.println("panHome=");
    Serial.println(panHomeFlag);
    Serial.println("tiltHome=");
    Serial.println(tiltHomeFlag);

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
    } 
    else {
    if(command != ""){
      handleCommand(command); 
    }
    command = "";
    }
  }

  panStepper.run();
  tiltStepper.run();
}
