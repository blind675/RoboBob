
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
