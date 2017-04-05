#include <EEPROM.h>
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

enum State { START, RUN_SEQUNECE, WAINT_FOR_POSITION, SAVE_POSITION };

typedef struct {
  byte servo1,servo2,servo3;
  bool isClawClosed;
} ServoStatus;

typedef struct {
  long timestamp;
  int sleepTime;
  bool isThreadAlive;
} Thread;

Servo ClawServo;
Servo servo1;
Servo servo2;
Servo servo3;

ServoStatus currentServoPoint = {0, 0, 0, false};

// define a thread for blinking led
Thread blinkThred = {0, 25, true};
int blinkLedState = LOW;

// define a main thread
Thread mainThread = {0, 0, true};
// define a thread for servo motors - not active
Thread servoThread = {0, 0, false};

State currentState = START;

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
  runServoThread();
  //  if (Serial.available() > 0) {
  //    int value = Serial.parseInt();
  //  }
}

// ********************* main thread ******************
void mainThreadSleep(int milliseconds) {
  mainThread.sleepTime += milliseconds;
}

void runMainThread() {
  if (mainThread.isThreadAlive && (millis() - mainThread.timestamp > mainThread.sleepTime )) {
    mainThread.sleepTime = 0; // reset delay

    switch (currentState) {
      case START:
        if (haveSavedSequence()) {
          currentState = RUN_SEQUNECE;
          turnLedOn();
        }
        break;
      case RUN_SEQUNECE:
        startServoThread();
        if (isClawButtonPressed() || isFunctionalButtonPressed()) {
          blinkLedSlow();
          stopServoThread();
          // TODO: clean all the EEPROM memory and wait for new instructions set
          currentState = WAINT_FOR_POSITION;
        }
        break;
      case WAINT_FOR_POSITION:
        if (isFunctionalButtonPressed()) {
          blinkLedFast();
          currentState = SAVE_POSITION;
        } else if (isClawButtonPressed()) {
          closeOpenClaw();
        }
        break;
      case SAVE_POSITION:
        // TODO: read servos positions and claw
        saveServoPositions();
        mainThreadSleep(200); // short delay on main thread
        if (isFunctionalButtonPressed()) {
          turnLedOn();
          currentState = RUN_SEQUNECE;
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
    servoThread.isThreadAlive = true;
    servoThread.timestamp = millis();
  }
}
void stopServoThread() {
  servoThread.isThreadAlive = false;
}
void runServoThread() {
  if (servoThread.isThreadAlive && (millis() - servoThread.timestamp > servoThread.sleepTime )) {
    // TODO: read servos step
    moveServos();
    servoThread.timestamp = millis();
  }
}

void moveServos() {
  // move all servos to the position in the
  servo1.write(currentServoPoint.servo1);
  servo2.write(currentServoPoint.servo2);
  servo3.write(currentServoPoint.servo3);
  if (currentServoPoint.isClawClosed) {
    ClawServo.write(0);
  } else {
    ClawServo.write(180);
  }

  servoThreadSleep(1000);
}
void servoThreadSleep(int milliseconds) {
  servoThread.sleepTime = milliseconds;
}

// ********************* claw ******************
void closeOpenClaw() {
  if (currentServoPoint.isClawClosed) {
    ClawServo.write(0);
  } else {
    ClawServo.write(180);
  }
  currentServoPoint.isClawClosed = !currentServoPoint.isClawClosed;

  mainThreadSleep(200); // short delay on main thread
}

// ********************* data ******************
bool haveSavedSequence() {
  // TODO: read first byte from EEPROM
  //  if not 0 return true
  return true;
}
void saveServoPositions() {
  // TODO: write the number of the step - like a counter 
  // and all the values from the structure
} //http://forum.arduino.cc/index.php?topic=155924.0
void readServoStep() {
  // TODO: read the first byte ( the counter )
  // and then the values 4 in the structure
  // if the first byte is 0 we are at the end
}

// TODO: function that puts all EEPROM to 0 

// ********************* blinking led thread **********
void runLedThread() {
  if ( blinkThred.isThreadAlive && ( millis() - blinkThred.timestamp > blinkThred.sleepTime) ) {

    digitalWrite(ledPin, blinkLedState);
    blinkLedState = !blinkLedState;
    blinkThred.timestamp = millis();
  }
}

void turnLedOn() {
  blinkLedThreadSleep(20);
}
void blinkLedSlow() {
  blinkLedThreadSleep(1000);
}
void blinkLedFast() {
  blinkLedThreadSleep(250);
}
void blinkLedThreadSleep(int milliseconds) {
  blinkThred.sleepTime = milliseconds;
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
