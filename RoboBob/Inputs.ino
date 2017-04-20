
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
  int servo1 = readServoForPin(servo1InPin);
  // do some servo corrections
  if ( servo1 > 200 ) {
    servo1 = 5;
  } else if ( servo1 > 180 ) {
    servo1 = 175;
  }
  currentServoPoint.servo1 = servo1;
    
  int servo2 = readServoForPin(servo2InPin);
  if ( servo2 > 200 ) {
    servo2 = 5;
  } else if ( servo2 > 180 ) {
    servo2 = 175;
  }
  currentServoPoint.servo2 = servo2;
  
  int servo3 = readServoForPin(servo3InPin);
  if ( servo3 > 200 ) {
    servo3 = 5;
  } else if ( servo3 > 180 ) {
    servo3 = 175;
  }
  currentServoPoint.servo3 = servo3;
  
  lastServoPoint = currentServoPoint;
}
int readServoForPin(int servoPin) {
  //close led
  digitalWrite(ledPin, LOW);
  //delay to stabilized
  delay(10);
  mainThreadSleep(10);
  //do 10 reads in an array
  Average<int> values(10);
  for(int i=0 ; i < 10 ; i++) {
    int readValue = analogRead(servoPin);
    values.push(readValue);
  }
  //get the value that is most found
  int baseValue = values.mode();
  // correnction trim the edges
  baseValue =  baseValue < 63 ? 63 : baseValue;
  baseValue =  baseValue > 662 ? 662 : baseValue;
  int basePosition = map(baseValue, 63, 662, 0, 180);
  return basePosition;
}

