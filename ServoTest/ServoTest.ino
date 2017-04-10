#include <Servo.h>

#define inputServoPin A2

int servoPin = 11;
int inputValue = 0;
Servo servoMain; // Define our Servo

void setup() {

    servoMain.attach(servoPin);
    Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps

    //  // set max reference to 2,56v
    //  analogReference(INTERNAL2V56);
}

void loop() {

        // send data only when you receive data:
        if (Serial.available() > 0) {
                inputValue = Serial.parseInt();

                // say what you got:
                Serial.print("Move servo to poz: ");
                Serial.println(inputValue);

                servoMain.write(inputValue);  // Turn Servo Left to 45 degrees
        }

   readServoForPin(inputServoPin);
   delay(1000);
}

int readServoForPin(int servoPin) {
  int baseValue = analogRead(servoPin);
  int basePosition = map(baseValue, 63, 662, 0, 180);

  Serial.print("-> Command position: ");
  Serial.print(inputValue);
  Serial.print(" read value: ");
  Serial.print(baseValue);
  Serial.print(" converted possition: ");
  Serial.println(basePosition);
  return basePosition;
}
