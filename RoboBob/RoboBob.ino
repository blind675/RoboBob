#include <Servo.h>

#define servo1Pin           11
#define servo1InPin         A2
#define servo2Pin           10
#define servo2InPin         A1
#define servo3Pin            9
#define servo3InPin         A0
#define clawServoPin         3
#define button1Pin           8      // functional button
#define button2Pin           4      // claw button
#define ledPin               2

#define servoDelayPerAngle   1

enum state {
  start,
  runSequence,
  waitForPosition,
  savePosition
};

Servo ClawServo;
Servo servo1;
Servo servo2;
Servo servo3;

bool clawClosed = false;

long lastLedChangedTimestamp = 0;
int blinkDelay = 25;
int blinkLedState = LOW;

long mainThreadTimestamp = 0;
int mainThreadDelay = 0; // none

state currentState = start;

void setup() {
  // inti all servos
  ClawServo.attach(clawServoPin);
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);

  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(ledPin, OUTPUT);

  //  // set max reference to 2,56v
  //  analogReference(INTERNAL2V56);

  Serial.begin(9600);
}

void loop() {

  runLedThread();
  runMainThread();

  if (Serial.available() > 0) {
    int value = Serial.parseInt();
  }
}

// ********************* main ******************
void runMainThread() {
  if ( millis() - mainThreadTimestamp > mainThreadDelay ) {
    mainThreadDelay = 0; // reset delay
    
    switch (currentState) {
      case start:
        if (haveSavedSequence()) {
          currentState = runSequence;
          turnLedOn();
        }
        break;
      case runSequence:
        // TODO: read servos step
        // TODO: move servos
        // TODO: delay ?? sau delay in move servos
        if (isClawButtonPressed() || isFunctionalButtonPressed()) {
          blinkLedSlow();
          currentState = waitForPosition;
        }
        break;
      case waitForPosition:
        if (isFunctionalButtonPressed()) {
          blinkLedFast();
          currentState = savePosition;
        } else if (isClawButtonPressed()) {
          closeOpenClaw();
        }
        break;
      case savePosition:
        // TODO: read servos positions and claw
        saveServoPositions();
        mainThreadDelay = 200; // short delay on main thread
        if (isFunctionalButtonPressed()) {
          turnLedOn();
          currentState = runSequence;
        } else {
          blinkLedSlow();
          currentState = waitForPosition;
        }
        break;
    }
    mainThreadTimestamp = millis();
  }
}

// ********************* data ******************
bool haveSavedSequence() {
  return true;
}
void saveServoPositions() {} //http://forum.arduino.cc/index.php?topic=155924.0
void readServoStep() {}

// ********************* blinking led **********
void runLedThread() {
  if ( millis() - lastLedChangedTimestamp > blinkDelay ) {

    digitalWrite(ledPin, blinkLedState);
    blinkLedState = !blinkLedState;
    lastLedChangedTimestamp = millis();
  }
}

void turnLedOn() {
  blinkDelay = 20;
}
void blinkLedSlow() {
  blinkDelay = 1000;
}
void blinkLedFast() {
  blinkDelay = 250;
}

// ********************* buttons ***************
bool readButtonStateForPin(int inPin) {
  if (digitalRead(inPin) == HIGH) {
    return true;
  } else {
    return false;
  }
}

bool isClawButtonPressed() {
  return readButtonStateForPin(button2Pin);
}
bool isFunctionalButtonPressed() {
  return readButtonStateForPin(button1Pin);
}

// ********************* claw ******************
void closeOpenClaw() {
  if (clawClosed) {
    ClawServo.write(0);
  } else {
    ClawServo.write(180);
  }
  clawClosed = !clawClosed;

  mainThreadDelay = 200; // short delay on main thread
}

//void readServos() {
//  int servoVal[3];
//
//  servoVal[0] = analogRead(servo2Input);
//  servoVal[1] = analogRead(servo3Input);
//  servoVal[2] = analogRead(servo4Input);
//
//  Serial.print("servo");
//
//  for (int i = 0; i < 3; i++) {
//    Serial.print(" ");
//    Serial.print(i);
//    Serial.print(" = ");
//    Serial.print(servoVal[i]);
//  }
//}

//void readBase() {
//
//  int baseValue = analogRead(baseInput);
//  int basePosition = map(baseValue, 270, 680, 0, 180);
//
//  Serial.print("   - base ");
//  Serial.print(" = ");
//  Serial.println(basePosition);
//}
