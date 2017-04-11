#include <EEPROM.h>
#include <Servo.h>

#define servo1Pin           11
#define servo1InPin         A2
#define servo2Pin           10
#define servo2InPin         A4
#define servo3Pin            9
#define servo3InPin         A3
#define clawServoPin         3
#define button1Pin           8      // functional button
#define button2Pin           4      // claw button
#define ledPin               2
#define EEPROMStartAddress   0

Servo ClawServo;
Servo servo1;
Servo servo2;
Servo servo3;

enum State { START, RUN_SEQUNECE, WAINT_FOR_POSITION, SAVE_POSITION };

typedef struct {
  byte servo1, servo2, servo3;
  bool isClawClosed;
} ServoPoint;

ServoPoint currentServoPoint = {0, 0, 0, false};
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
  writeEEPROMValuesDEBUG();
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
        Serial.println("**** START state ****");
        if (haveSavedSequence()) {
          readServoStepsFromEEPROM();
          currentState = RUN_SEQUNECE;
          turnLedOn();
        } else {
          currentState = WAINT_FOR_POSITION;
          stopServoThread();
        }
        break;
      case RUN_SEQUNECE:
        Serial.println("**** RUN SEQUNECE state ****");
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
        Serial.println("**** WAINT FOR POSITION state ****");
        if (isFunctionalButtonPressed()) {
          blinkLedFast();
          currentState = SAVE_POSITION;
        } else if (isClawButtonPressed()) {
          closeOpenClaw();
        }
        break;
      case SAVE_POSITION:
        Serial.println("**** SAVE POSITION state ****");
        readServosPossition();
        // add the currentServoPoint to the array;
        currentStepCount += 1;
        totalStepsCount +=1;
        if ( currentStepCount == 21 ) {
          currentStepCount = 1;
          saveServoPositionsToEEPROM();
          turnLedOn();
          currentState = RUN_SEQUNECE;
          mainThreadSleep(500);
        } else {
          allPoints[currentStepCount-1] = currentServoPoint;
        }
        printCurentServoPointsDEBUG();
        mainThreadSleep(500); // short delay on main thread
        if (isFunctionalButtonPressed()) {
          currentStepCount = 1;
          saveServoPositionsToEEPROM();
          turnLedOn();
          currentState = RUN_SEQUNECE;
          mainThreadSleep(500);
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
  // this is not he correct possition but i can't find a better one
  detachAllServos();
}
void runServoThread() {
  if (servoThread.isThreadAlive && (millis() - servoThread.timestamp > servoThread.sleepTime )) {
    currentStepCount +=1;
    if ( currentStepCount == totalStepsCount + 1) {
      currentStepCount = 1;
    }
    currentServoPoint = allPoints[currentStepCount - 1];
    printCurentServoPointsDEBUG();
    moveServos();
    servoThread.timestamp = millis();
  }
}
void servoThreadSleep(int milliseconds) {
  servoThread.sleepTime = milliseconds;
}

void moveServos() {
  servo1.write(currentServoPoint.servo1);
  servo2.write(currentServoPoint.servo2);
  servo3.write(currentServoPoint.servo3);
  if (currentServoPoint.isClawClosed) {
    ClawServo.write(0);
  } else {
    ClawServo.write(180);
  }
  servoThreadSleep(1500);
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
// ********************* claw ******************
void closeOpenClaw() {
  if (currentServoPoint.isClawClosed) {
    ClawServo.write(55);
  } else {
    ClawServo.write(170);
  }
  currentServoPoint.isClawClosed = !currentServoPoint.isClawClosed;
  mainThreadSleep(200); // short delay on main thread ... why main ??
}

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

// ********************* read ***************
void readServosPossition() {
  //read first servo position
  int servo1 = readServoForPin(servo1InPin);
  // do some servo corrections
  if ( servo1 > 200 ) {
    servo1 = 5;
  } else if ( servo1 > 180 ) {
    servo1 = 175;
  }
  if ( servo1 < currentServoPoint.servo1 - 5 || servo1 > currentServoPoint.servo1 + 5 ) {
    currentServoPoint.servo1 = servo1;
  }
  int servo2 = readServoForPin(servo2InPin);
  if ( servo2 > 200 ) {
    servo2 = 5;
  } else if ( servo2 > 180 ) {
    servo2 = 175;
  }
  if ( servo2 < currentServoPoint.servo2 - 5 || servo2 > currentServoPoint.servo2 + 5 ) {
    currentServoPoint.servo2 = servo2;
  }
  int servo3 = readServoForPin(servo3InPin);
  if ( servo3 > 200 ) {
    servo3 = 5;
  } else if ( servo3 > 180 ) {
    servo3 = 175;
  }
  if ( servo3 < currentServoPoint.servo3 - 5 || servo3 > currentServoPoint.servo3 + 5 ) {
    currentServoPoint.servo3 = servo3;
  }
}
int readServoForPin(int servoPin) {
  int baseValue = analogRead(servoPin);
  int basePosition = map(baseValue, 63, 662, 0, 180);
  return basePosition;
}

// ********************* data ******************
bool haveSavedSequence() {
  // read first byte from EEPROM
  // if not 0 return true
  totalStepsCount = EEPROM.read(EEPROMStartAddress);
  if (totalStepsCount == 0) {
    return false;
  } else {
    return true;
  }
}
void saveServoPositionsToEEPROM() {
  // write all to EEPROM
  // start with the totalStepCount
  EEPROM.write(EEPROMStartAddress, totalStepsCount);
  byte EEPROMIndex = EEPROMStartAddress + 1;
  for (byte i = 0; i < totalStepsCount ; i++) {
    EEPROM.write(EEPROMIndex, allPoints[i].servo1); EEPROMIndex += 1;
    EEPROM.write(EEPROMIndex, allPoints[i].servo2); EEPROMIndex += 1;
    EEPROM.write(EEPROMIndex, allPoints[i].servo3); EEPROMIndex += 1;
    EEPROM.write(EEPROMIndex, allPoints[i].isClawClosed); EEPROMIndex += 1;
  }
}
void readServoStepsFromEEPROM() {
  // read all the steps 4 in to the structure
  byte EEPROMIndex = EEPROMStartAddress + 1;
  for (byte i = 0; i < totalStepsCount ; i++) {
    allPoints[i].servo1 = EEPROM.read(EEPROMIndex); EEPROMIndex += 1;
    allPoints[i].servo2 = EEPROM.read(EEPROMIndex); EEPROMIndex += 1;
    allPoints[i].servo3 = EEPROM.read(EEPROMIndex); EEPROMIndex += 1;
    allPoints[i].isClawClosed = EEPROM.read(EEPROMIndex); EEPROMIndex += 1;
  }
}
void clearEEPROM() {
  // clear ony the counter , first byte
  EEPROM.write(EEPROMStartAddress, 0);
}


// ********************* debug ***************
void printCurentServoPointsDEBUG() {
  Serial.println("ServoPointsStruct");
  Serial.print("  servo1 :");
  Serial.println(currentServoPoint.servo1);
  Serial.print("  base servo :");
  Serial.println(currentServoPoint.servo2);
  Serial.print("  servo 3 :");
  Serial.println(currentServoPoint.servo3);
  Serial.print("  isClawClosed :");
  Serial.println(currentServoPoint.isClawClosed);
  Serial.print(currentStepCount);
  Serial.print(" / ");
  Serial.println(totalStepsCount);
}

void readServosPossitionDEBUG() {
  if (Serial.available() > 0) {
    String str = Serial.readString();// read the incoming data as string
    if (str[0] == 'x') {
      str.remove(0, 2);
      int value = str.toInt();
      currentServoPoint.servo1 = value;
    } else if (str[0] == 'y') {
      str.remove(0, 2);
      int value = str.toInt();
      currentServoPoint.servo2 = value;
    } else if (str[0] == 'z') {
      str.remove(0, 2);
      int value = str.toInt();
      currentServoPoint.servo3 = value;
    } else {
      printCurentServoPointsDEBUG();
    }
  }
}

void writeEEPROMValuesDEBUG() {
  int EEPROMaddressDEBUG = 0;

  // 6 steps
  EEPROM.write(EEPROMaddressDEBUG, 6); EEPROMaddressDEBUG += 1;
  // 1
  EEPROM.write(EEPROMaddressDEBUG, 0 ); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  // 2
  EEPROM.write(EEPROMaddressDEBUG, 0 ); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;
  // 3
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 60); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;
  // 4
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 60); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 180); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;
  // 5
  EEPROM.write(EEPROMaddressDEBUG, 180); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 180); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;
  // 6
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0);
}
