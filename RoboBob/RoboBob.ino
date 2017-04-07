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
  byte stepCount, servo1, servo2, servo3;
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

int blinkLedState = LOW;
// define a thread for blinking led
Thread blinkThred = {0, 25, true};
// define a main thread
Thread mainThread = {0, 0, true};
// define a thread for servo motors - not active
Thread servoThread = {0, 0, false};

int EEPROMAddress = 0;
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
          currentState = RUN_SEQUNECE;
          turnLedOn();
        } else {
          currentState = WAINT_FOR_POSITION;
        }
        break;
      case RUN_SEQUNECE:
        Serial.println("**** RUN SEQUNECE state ****");
        startServoThread();
        if (isClawButtonPressed() || isFunctionalButtonPressed()) {
          blinkLedSlow();
          stopServoThread();
          clearEEPROM();
          currentState = WAINT_FOR_POSITION;
        }
        break;
      case WAINT_FOR_POSITION:
        Serial.println("**** WAINT FOR POSITION state ****");     

        readServosPossitionDEBUG();
          
        if (isFunctionalButtonPressed()) {
          blinkLedFast();
          currentState = SAVE_POSITION;
        } else if (isClawButtonPressed()) {
          closeOpenClaw();
        }
        break;
      case SAVE_POSITION:
        Serial.println("**** SAVE POSITION state ****");
        // TODO: uncomment in production
        // readServosPossition();
        if (saveServoPositionsToEEPROM() == false) {
          // full EEPROM .. what arte the odds
          // go to start .. i guess
          currentState = START;
        } 
        
        Serial.print(" Written structure to the address: ");
        Serial.println(EEPROMAddress);
        printCurentServoPointsDEBUG();
        
        mainThreadSleep(500); // short delay on main thread
        if (isFunctionalButtonPressed()) {
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
    EEPROMAddress = 0;
    servoThread.isThreadAlive = true;
    servoThread.timestamp = millis();
  }
}
void stopServoThread() {
  servoThread.isThreadAlive = false;
  // this is not he correct possition but i can't find a better one
  currentServoPoint.stepCount = 0;
  EEPROMAddress = 0;
}
void runServoThread() {
  if (servoThread.isThreadAlive && (millis() - servoThread.timestamp > servoThread.sleepTime )) {
    if ( readServoStepFromEEPROM() == false ) {
      // reached the end - go to the begining of the EEPROM
      EEPROMAddress = 0;
      readServoStepFromEEPROM();
    }
    moveServos();
    servoThread.timestamp = millis();
  }
}

void moveServos() {
  
  Serial.print(" + Current eeprom address: ");
  Serial.println(EEPROMAddress);
  Serial.println(" + Move servos to positions:");
  printCurentServoPointsDEBUG();
  
  // move all servos to the position in the
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
  mainThreadSleep(200); // short delay on main thread ... why main ??
}

// ********************* data ******************
bool haveSavedSequence() {
  // read first byte from EEPROM
  // if not 0 return true
  EEPROMAddress = 0;
  return readServoStepFromEEPROM();
}
bool saveServoPositionsToEEPROM() {
  // write the number of the step - like a counter
  // and all the values from the structure
  currentServoPoint.stepCount += 1;
  EEPROM.write(EEPROMAddress, currentServoPoint.stepCount); EEPROMAddress += 1;
  EEPROM.write(EEPROMAddress, currentServoPoint.servo1); EEPROMAddress += 1;
  EEPROM.write(EEPROMAddress, currentServoPoint.servo2); EEPROMAddress += 1;
  EEPROM.write(EEPROMAddress, currentServoPoint.servo3); EEPROMAddress += 1;
  EEPROM.write(EEPROMAddress, currentServoPoint.isClawClosed); EEPROMAddress += 1;
  
  if (EEPROMAddress > 100) {
    EEPROMAddress = 0;
    return false;
  }
  return true;
}
bool readServoStepFromEEPROM() {
  // read the first byte ( the counter )
  // and then the values 4 in the structure
  // if the first byte is 0 we are at the end
  byte stepCount = EEPROM.read(EEPROMAddress); EEPROMAddress += 1;
  if (stepCount == 0) {
    EEPROMAddress = 0;
    return false;
  } else {
    currentServoPoint.stepCount = stepCount;
  }
  currentServoPoint.servo1 = EEPROM.read(EEPROMAddress); EEPROMAddress += 1;
  currentServoPoint.servo2 = EEPROM.read(EEPROMAddress); EEPROMAddress += 1;
  currentServoPoint.servo3 = EEPROM.read(EEPROMAddress); EEPROMAddress += 1;
  currentServoPoint.isClawClosed = EEPROM.read(EEPROMAddress); EEPROMAddress += 1;
  return true;
}
// function that puts all EEPROM to 0
void clearEEPROM() {
  // only use the first100 bytes of eeprom 
  // to prevent memory use in time
  for (int i = 0 ; i < 104 ; i++) {
    EEPROM.write(i, 0);
  }
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
  if ( servo1 < currentServoPoint.servo1 - 5 || servo1 > currentServoPoint.servo1 + 5 ) {
    currentServoPoint.servo1 = servo1;
  }
  int servo2 = readServoForPin(servo2InPin);
  if ( servo2 < currentServoPoint.servo2 - 5 || servo2 > currentServoPoint.servo2 + 5 ) {
    currentServoPoint.servo2 = servo2;
  }
  int servo3 = readServoForPin(servo3InPin);
  if ( servo3 < currentServoPoint.servo3 - 5 || servo3 > currentServoPoint.servo3 + 5 ) {
    currentServoPoint.servo3 = servo3;
  }
}
int readServoForPin(int servoPin) {
  int baseValue = analogRead(servoPin);
  int basePosition = map(baseValue, 270, 680, 0, 180);
  return basePosition;
}

// ********************* debug ***************
void printCurentServoPointsDEBUG() {
  Serial.println("ServoPointsStruct");
    Serial.print("  stepCount :");
    Serial.println(currentServoPoint.stepCount);
    Serial.print("  servo1 :");
    Serial.println(currentServoPoint.servo1);
    Serial.print("  servo2 :");
    Serial.println(currentServoPoint.servo2);
    Serial.print("  servo3 :");
    Serial.println(currentServoPoint.servo3);
    Serial.print("  isClawClosed :");
    Serial.println(currentServoPoint.isClawClosed);
}

void readServosPossitionDEBUG() {
  if (Serial.available() > 0) {
    String str = Serial.readString();// read the incoming data as string
    if (str[0] == 'x') {
        str.remove(0,2);
      int value = str.toInt();
        currentServoPoint.servo1 = value;
    } else if (str[0] == 'y') {
        str.remove(0,2);
      int value = str.toInt();
        currentServoPoint.servo2 = value;
    } else if (str[0] == 'z') {
        str.remove(0,2);
      int value = str.toInt();
        currentServoPoint.servo3 = value;
    } else {
      printCurentServoPointsDEBUG();
    }
  }
}

void writeEEPROMValuesDEBUG() {
  int EEPROMaddressDEBUG = 0;

  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0 ); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;

  EEPROM.write(EEPROMaddressDEBUG, 2); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0 ); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;

  EEPROM.write(EEPROMaddressDEBUG, 3); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 60); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;

  EEPROM.write(EEPROMaddressDEBUG, 4); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 60); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 180); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;

  EEPROM.write(EEPROMaddressDEBUG, 5); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 180); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 180); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 1); EEPROMaddressDEBUG += 1;

  EEPROM.write(EEPROMaddressDEBUG, 6); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 90); EEPROMaddressDEBUG += 1;
  EEPROM.write(EEPROMaddressDEBUG, 0); EEPROMaddressDEBUG += 1;
  
}
