
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

// ********************* read servos positions ***************
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

