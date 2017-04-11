
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
