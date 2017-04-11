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
