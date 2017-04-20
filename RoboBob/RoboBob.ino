#include <EEPROM.h>
#include <Servo.h>
#include <Average.h>

#define servo1Pin           11
#define servo1InPin         A2
#define servo2Pin           10
#define servo2InPin         A3
#define servo3Pin            9
#define servo3InPin         A4
#define clawServoPin         3
#define button1Pin           8      // functional button
#define button2Pin           4      // claw button
#define ledPin               2
#define EEPROMStartAddress   0
#define SERVO_DELAY         25

Servo ClawServo;
Servo servo1;
Servo servo2;
Servo servo3;

enum State { START, RUN_SEQUNECE, WAINT_FOR_POSITION, SAVE_POSITION };

typedef struct {
  byte servo1, servo2, servo3;
  bool isClawClosed;
} ServoPoint;

ServoPoint currentServoPoint = {90, 90, 90, false};
ServoPoint lastServoPoint = {90, 90, 90, false};
byte totalStepsCount = 0;
byte currentStepCount = 0;
ServoPoint allPoints[20];

typedef struct {
  long timestamp;
  int sleepTime;
  bool isThreadAlive;
} Thread;

int blinkLedState = LOW;
// define a thread for blinking led
Thread blinkThred = {0, 25, true};
// define a main thread
Thread mainThread = {0, 0, true};
// define a thread for servo motors - not active
Thread servoThread = {0, 0, false};

int EEPROMAddress = EEPROMStartAddress;
State currentState = START;

void setup() {
  // inti all servos
  ClawServo.attach(clawServoPin);
  attachAllServos();

  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  // generate some debug data and write it to EEPROM
  //  writeEEPROMValuesDEBUG();

  EEPROM.write(0, 0);
}

void loop() {
  runLedThread();
  runServoThread();
  runMainThread();
}

// ********************* main thread ******************
void mainThreadSleep(int milliseconds) {
  mainThread.sleepTime += milliseconds;
}
void runMainThread() {
  if (mainThread.isThreadAlive && (millis() - mainThread.timestamp > mainThread.sleepTime )) {
    mainThread.sleepTime = 100; // reset delay - some delay so print looks ok

    switch (currentState) {
      case START:
        //        Serial.println("**** START state ****");
        if (haveSavedSequence()) {
          readServoStepsFromEEPROM();
          currentState = RUN_SEQUNECE;
          turnLedOn();
        } else {
          blinkLedSlow();
          currentState = WAINT_FOR_POSITION;
          stopServoThread();
        }
        break;
      case RUN_SEQUNECE:
        //        Serial.println("**** RUN SEQUNECE state ****");
        startServoThread();
        if (isClawButtonPressed() || isFunctionalButtonPressed()) {
          blinkLedSlow();
          stopServoThread();
          clearEEPROM();
          totalStepsCount = 0;
          currentStepCount = 0;
          currentState = WAINT_FOR_POSITION;
        }
        break;
      case WAINT_FOR_POSITION:
        //        Serial.println("**** WAINT FOR POSITION state ****");
        if (isFunctionalButtonPressed()) {
          blinkLedFast();
          currentState = SAVE_POSITION;
        } else if (isClawButtonPressed()) {
          closeOpenClaw();
        }
        break;
      case SAVE_POSITION:
        //        Serial.println("**** SAVE POSITION state ****");
        readServosPossition();
        // add the currentServoPoint to the array;
        currentStepCount += 1;
        totalStepsCount += 1;
        if ( currentStepCount == 21 ) {
          currentStepCount = 1;
          saveServoPositionsToEEPROM();
          turnLedOn();
          currentState = RUN_SEQUNECE;
          mainThreadSleep(500);
        } else {
          allPoints[currentStepCount - 1] = currentServoPoint;
        }
        printCurentServoPointsDEBUG();
        mainThreadSleep(1500); // short delay on main thread
        if (isFunctionalButtonPressed()) {
          currentStepCount = 1;
          saveServoPositionsToEEPROM();
          turnLedOn();
          currentState = RUN_SEQUNECE;
          mainThreadSleep(1000);
        } else {
          blinkLedSlow();
          currentState = WAINT_FOR_POSITION;
        }
        break;
    }
    mainThread.timestamp = millis();
  }
}

// ********************* servos thread ****************
void startServoThread() {
  if (servoThread.isThreadAlive == false) {
    attachAllServos();
    servoThread.isThreadAlive = true;
    servoThread.timestamp = millis();
  }
}
void stopServoThread() {
  servoThread.isThreadAlive = false;
  detachAllServos();
}
void runServoThread() {
  if (servoThread.isThreadAlive && (millis() - servoThread.timestamp > servoThread.sleepTime )) {
    currentStepCount += 1;
    if ( currentStepCount == totalStepsCount + 1) {
      currentStepCount = 1;
    }
    currentServoPoint = allPoints[currentStepCount - 1];
    moveServos();
    servoThread.timestamp = millis();
  }
}
void servoThreadSleep(int milliseconds) {
  servoThread.sleepTime = milliseconds;
}
void detachAllServos() {
  if (servo1.attached()) servo1.detach();
  if (servo2.attached()) servo2.detach();
  if (servo3.attached()) servo3.detach();
}
void attachAllServos() {
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
}

void moveServos() {
  printCurentServoPointsDEBUG();
  // TODO: refactor this in a decent code
  if (lastServoPoint.servo1 != currentServoPoint.servo1) {
    if (lastServoPoint.servo1 < currentServoPoint.servo1 )  {
      for ( byte x = lastServoPoint.servo1 + 1; x < currentServoPoint.servo1; x++) {
        servo1.write(x);
        delay(SERVO_DELAY);
      }
    } else {
      for ( byte x = lastServoPoint.servo1 - 1; x > currentServoPoint.servo1; x--) {
        servo1.write(x);
        delay(SERVO_DELAY);
      }
    }
  }

  if (lastServoPoint.servo2 != currentServoPoint.servo2) {
    if (lastServoPoint.servo2 < currentServoPoint.servo2)  {
      for ( byte y = lastServoPoint.servo2 + 1; y < currentServoPoint.servo2; y++) {
        servo2.write(y);
        delay(SERVO_DELAY);
      }
    } else {
      for ( byte y = lastServoPoint.servo2 - 1; y > currentServoPoint.servo2; y--) {
        servo2.write(y);
        delay(SERVO_DELAY);
      }
    }
  }

  if (lastServoPoint.servo3 != currentServoPoint.servo3) {
    if (lastServoPoint.servo3 < currentServoPoint.servo3 )  {
      for ( byte z = lastServoPoint.servo3 + 1; z < currentServoPoint.servo3; z++) {
        servo3.write(z);
        delay(SERVO_DELAY);
      }
    } else {
      for ( byte z = lastServoPoint.servo3 - 1; z > currentServoPoint.servo3; z--) {
        servo3.write(z);
        delay(SERVO_DELAY);
      }
    }
  }

  if (lastServoPoint.isClawClosed != currentServoPoint.isClawClosed) {
    if (currentServoPoint.isClawClosed) {
      for (byte i = 55; i < 150; i++) {
        ClawServo.write(i);
        delay(SERVO_DELAY);
      }
    } else {
      for (byte i = 150; i > 55; i--) {
        ClawServo.write(i);
        delay(SERVO_DELAY);
      }
    }
  }

  lastServoPoint = currentServoPoint;
  delay(100);
  mainThreadSleep(400);
  servoThreadSleep(400);
}

// ********************* claw ******************
void closeOpenClaw() {
  if (currentServoPoint.isClawClosed) {
    for (byte i = 150; i > 55; i--) {
      ClawServo.write(i);
      delay(SERVO_DELAY);
    }
    Serial.println("CLOSED");
  } else {
    for (byte i = 55; i < 150; i++) {
      ClawServo.write(i);
      delay(SERVO_DELAY);
    }
    Serial.println("OPEN");
  }
  currentServoPoint.isClawClosed = !currentServoPoint.isClawClosed;
  //  mainThreadSleep(2500);
  //  servoThreadSleep(2500);
}
