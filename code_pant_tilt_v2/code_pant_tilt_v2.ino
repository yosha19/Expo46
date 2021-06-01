#include <AccelStepper.h>
int count = 1;
int array[] = {0};
int count2 = 1;
int array2[] = {0};
//#define Y_nEN  (24)      //Or A0 for analog (a7)
//#define Y_OPTO  (25)    //Or A1 for analog (a6)
//#define THERM (26)      //Or A2 for analog (a5)
//#define Z_STEP  (27)    //Or A3 for analog (a4)
//#define Z_DIR  (28)     //Or A4 for analog (a3)
//#define Z_nEN  (29)      //Or A5 for analog (a2)
//#define Z_OPTO  (30)    //Or A6 for analog (a1)
//#define E_STEP  (4)     //b4
//#define E_nEN  (3)       //b3
//#define E_DIR  (2)      //b2
//#define Y_STEP  (23)    //c7
//#define Y_DIR (22)      //c6
//#define X_OPTO  (20)    //c4
//#define X_nEN  (19)      //c3
//#define X_DIR  (18)     //c2
//#define SDA  (17)       //c1
//#define SCL  (16)       //c0
//#define X_STEP  (15)    //d7
//#define HEAT  (14)      //d6


//Pin definitions when using Arduino Nano pinout
#define BAUD_RATE 115200
#define PIN_PAN_HALL  12
#define PIN_TILT_HALL  13
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

#define PAN_GEAR_RATIO (144.0/17.0) //44/17  // teeth
#define TILT_GEAR_RATIO (64.0/21.0) // teeth

AccelStepper panStepper = AccelStepper(AccelStepper::DRIVER, PIN_STEP_PAN, PIN_DIRECTION_PAN);
AccelStepper tiltStepper = AccelStepper(AccelStepper::DRIVER, PIN_STEP_TILT, PIN_DIRECTION_TILT);


float pan_steps_per_degree = (400.0 * 8.0 * PAN_GEAR_RATIO) / 360.0 / 2.0; //Stepper motor has 200 steps per 360 degrees, dus 200/360 geeft aantal stappen per graad, deel je dit nog eens door 2 heb je het aantal stappen per halve graad.
//(begin regel hierboven met lezen) Het kleine tandwiel moet sneller draaien dan het grote tandwiel, waarvoor de verhouding van 144/17 geldt.
//Maar andersom gekeken zal het grote tandwiel dus langzamer moeten draaien dus geldt de verhouding van 17/144.
//De motor stuurt het kleine tandwiel aan waardoor de verhouding van 144/17 geldt. Ditzelfde principe geldt voor de motor voor tilting.
float tilt_steps_per_degree = (400.0 * 8.0 * TILT_GEAR_RATIO) / 360.0 / 2.0; //Stepper motor has 200 steps per 360 degrees
float pan_max_speed = 200; //degrees/second. Deze kan dus aangepast worden naar hoelang de beweging mag duren. De eis was om binnen 4 sec 45 graden te halen, en dat haal je hier wel mee.
float tilt_max_speed = 200; //degrees/second.
bool panHomeFlag = false;
bool tiltHomeFlag = false;
int panHomingDir = -1;
int tiltHomingDir = -1;
int hallState1 = 0;
int hallState2 = 0;

void setup() {
  //panStepper->setEnablePin(Z_nEN);
  //tiltStepper->setEnablePin(X_nEN);
  panStepper.setPinsInverted( false, false, false, false, true);
  tiltStepper.setPinsInverted( false, false, false, false, true);
  //  panStepper->enableOutputs();
  //  tiltStepper->enableOutputs();
  //pinMode(X_nEN, OUTPUT);
  //digitalWrite(X_nEN, LOW);
  Serial.begin(BAUD_RATE);
  pinMode(PIN_MS1, OUTPUT);
  pinMode(PIN_MS2, OUTPUT);
  //pinMode(Z_nEN, OUTPUT);
  //pinMode(Y_nEN, OUTPUT);
  pinMode(PIN_DIRECTION_PAN, OUTPUT);
  pinMode(PIN_STEP_PAN, OUTPUT);
  pinMode(PIN_DIRECTION_TILT, OUTPUT);
  pinMode(PIN_STEP_TILT, OUTPUT);
  pinMode(PIN_PAN_HALL, INPUT_PULLUP);
  pinMode(PIN_TILT_HALL, INPUT_PULLUP);


  panStepper.setAcceleration(50000);
  panStepper.setMaxSpeed(panDegreesToSteps(pan_max_speed));
  tiltStepper.setAcceleration(50000);
  tiltStepper.setMaxSpeed(tiltDegreesToSteps(tilt_max_speed));
  //digitalWrite(Z_nEN, LOW); //Enable the stepper drivers
  //digitalWrite(Y_nEN, LOW);
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
    //    array[count] = target1;
    //    count = count + 1;
    panStepper.moveTo(target);
    Serial.print("p_target=");
    Serial.println(target);

  } else if (command == "p_target?") {
    Serial.print("p_target=");
    Serial.println(panStepsToDegrees(panStepper.targetPosition()));

  } else if (command.startsWith("t_target=")) {
    long target = tiltDegreesToSteps(testioGetFloat(command));
    //    array2[count2] = target2;
    //    count2 = count2 + 1;

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
      panStepper.enableOutputs();
      Serial.println("p_enable=1");
    } else {
      panStepper.disableOutputs();
      Serial.println("p_enable=0");
    }


  } else if (command == "autohome?") {
    //panHome();
    //tiltHome();
    homeStepper(panStepper, PIN_PAN_HALL, 0.0, panDegreesToSteps(720.0));
    homeStepper(tiltStepper, PIN_TILT_HALL, -5.6, tiltDegreesToSteps(720.0));
    Serial.println("autohome=1");




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
    Serial.println("panHome=");
    Serial.println(panHomeFlag);
    Serial.println("tiltHome=");
    Serial.println(tiltHomeFlag);

  }
  //   else if (command == ("start")) {
  //    for (int i = 0; i <= count; i++) {
  //      panStepper->moveTo(array[i]);
  //    }
  //    for (int i = 0; i <= count2; i++) {
  //      tiltStepper->moveTo(array2[i]);
  //
  //    }
  //  }else {
  //    Serial.print("Unknown command: ");
  //    Serial.println(command);
  //
  //  }
  //}
}
String command;
void loop() {

  if (Serial.available()) {
    char c = Serial.read();
    if (c != '\r' && c != '\n') {
      command += c;
    }
    else {
      if (command != "") {
        handleCommand(command);
      }
      command = "";
    }
  }
  Serial.print("digitalRead(PIN_PAN_HALL)=");
  Serial.print(digitalRead(PIN_PAN_HALL));
  Serial.print(", digitalRead(PIN_TILT_HALL)=");
  Serial.println(digitalRead(PIN_TILT_HALL));
    panStepper.run();
    tiltStepper.run();
//  pinMode(PIN_PAN_HALL, INPUT);
//  pinMode(PIN_TILT_HALL, INPUT);



}
