// PWM Solar charge controller using Julian Iletts hardware design and code (https://www.youtube.com/playlist?list=PLjzGSu1yGFjUsB3ChZJ2_-MTNp7bHaKED)
// but adapted to work on the ATTiny25/45/85 by Adam Welch (https://www.youtube.com/channel/UCm5sG3-BXQZfVy3st2T_XKg).
// In arduino IDE you will need to add the following to your Additional Board Manager URLs within Preferences:
// https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
// This code should work on all ATTiny25/45/85s but make sure to select the right one when uploading your code.
// To get thre PWM outputs on the ATTiny*5 (and set them reliably) I found these sites super useful:
// https://forum.arduino.cc/index.php?topic=134754.msg1013479#msg1013479
// http://www.technoblogy.com/show?LE0

const float vFloat = 13.5;  //Battery float voltage - Change this to your desired level.
const float vReg = 5.0;  //Real value of 5 volt output from regulator - check this value if possible it will make the float voltage above more accurate.
const int R1 = 82000; //Value of R1 in Ohms - Check Schematic - https://github.com/AdamWelchUK/PWM85/
const int R2 = 20000; //Value or R2 in Ohms
const int setPoint = vFloat * (R2 / (R2+R1)) * (1024 / vReg);  //Set target for modulation.
int measurement = 0;
int pulseWidth = 0;
int difference = 0;
int stepSize = 0;
 
void setup() {
   
    TCCR0A = 2<<COM0A0 | 3<<COM0B0 | 3<<WGM00;  // Timer 0 Control Register A - Set None Inverting Mode in Fast PWM, Set Inverting Mode in Fast PWM, Enable Fast PWM (11.9.2 in ATTiny85 datasheet)
    TCCR0B = 0<<WGM02 | 1<<CS00;  // Timer 0 Control Register B - Enable Fast PWM, Clock Select Bit no prescaling (11.9.3 in ATTiny85 datasheet)
    TCCR1 = 6<<CS10;  // Timer 1 Control Register -  Set Prescaler (1<<CS10 ~ 4kHz, 2 ~ 2kHz, 3 ~ 1kHz, 4 ~ 500Hz, 5 ~ 250Hz, 6 ~ 125Hz, 7 ~ 63Hz)
    GTCCR = 1<<PWM1B | 2<<COM1B0;  // General Control Register for Timer 1 - Enable use of pin OC1B, None inverting mode.
   
  analogWrite(0, 117); //ATTiny85 Pin 5 // OC0A
  analogWrite(1, 137); //ATTiny85 Pin 6 // OC1B
  digitalWrite(3, HIGH); //ATTiny Pin 2
  delay(2000);
  digitalWrite(3, LOW);  // Flash onboard LED to show circuit is working.
}
 
void loop() {
  measurement = analogRead(A1); //ATTiny85 Pin 7
  difference = abs(setPoint - measurement);
  stepSize = difference;
   
  if (measurement < setPoint)
  {
    pulseWidth += stepSize;
    if (pulseWidth > 255) pulseWidth = 255;
  }
  else if (measurement > setPoint)
  {
    pulseWidth -= stepSize;
    if (pulseWidth < 0) pulseWidth = 0;
  }
  analogWrite(4, pulseWidth); //ATTiny Pin 3 
  delay(10);
  }
