/* PWM Solar charge controller using Julian Iletts hardware design and code (https://www.youtube.com/playlist?list=PLjzGSu1yGFjUsB3ChZJ2_-MTNp7bHaKED)
  but adapted to work on the ATTiny25/45/85 by Adam Welch (http://adamwelch.co.uk).
  In arduino IDE you will need to add the following to your Additional Board Manager URLs within Preferences:
  https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json

  For the highest efficiency you should change the ATTiny to an internal 1MHz clock speed -
  to apply this setting click Burn Bootloader in the Arduino IDE.

  This code should work on all ATTiny25/45/85s but make sure to select the right one when uploading your code.
  To get thre PWM outputs on the ATTiny*5 (and set them reliably) I found these sites super useful:
  https://forum.arduino.cc/index.php?topic=134754.msg1013479#msg1013479
  http://www.technoblogy.com/show?LE0

                ATMEL ATTiny85
                     +-\/-+
               RST  1|    |8  VCC
     LED (D 3) PB3  2|    |7  PB2 (A 1) SCK
  DRIVER (D 4) PB4  3|    |6  PB1 (D 1) MISO
               GND  4|    |5  PB0 (D 0) MOSI
                     +----+

 Fuse Settings : Low=0x62, High=0xDF, Ext=0xFF, Lock= 0xFF (1MHz Clock Bootloader)
 Seems no software BOD disable is available on my ATtiny version (fairly common)
*/

#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define wdt_int() WDTCR |= _BV(WDIE) // | _BV(WDCE) | _BV(WDE) // WDT goes to interrupt, not reset

/*Battery Charging Set Point.

  This equation is calculated at compile time to set the interger which the charge controller uses as it's modulation set point.
  By default this is set to a value to charge the battery to 13.5 volts.  Change this number if you wish. Change this number if you wish - absolute maximum 16.8v
  
  It is also reconmended to test the output of the regulator and use this equation to calibrate the charge controller.

  floatVoltage = battery float voltage (13.5 volts by default)
  regulatorVoltage = output from the voltage regulator (5.0 or 3.3 volts typically - but check yours!)
  R1 = value of R1 in the circuit (82000 ohms by design)
  R2 = value of R2 in the circuit (20000 ohms by design)
*/

#define floatVoltage 13.5
#define regulatorVoltage 3.3
#define R1 82000
#define R2 20000

const int setPoint = floatVoltage * R2 / (R2 + R1) * 1024 / regulatorVoltage;

int measurement = 0;
int pulseWidth = 0;
int stepSize = 0;

void setup() {

  pinMode(0, OUTPUT); // MOSI - PUMP1
  pinMode(1, OUTPUT); // MISO - PUMP2
  pinMode(2, INPUT);  // SCK  - MES
  pinMode(3, OUTPUT); // LED
  pinMode(4, OUTPUT); // DRIVER

  TCCR0A = 2 << COM0A0 | 3 << COM0B0 | 3 << WGM00; // Timer 0 Control Register A - Set None Inverting Mode in Fast PWM, Set Inverting Mode in Fast PWM, Enable Fast PWM (11.9.2 in ATTiny85 datasheet)
  TCCR0B = 0 << WGM02 | 1 << CS00; // Timer 0 Control Register B - Enable Fast PWM, Clock Select Bit no prescaling (11.9.3 in ATTiny85 datasheet)
  TCCR1 = 6 << CS10; // Timer 1 Control Register -  Set Prescaler (1<<CS10 ~ 4kHz, 2 ~ 2kHz, 3 ~ 1kHz, 4 ~ 500Hz, 5 ~ 250Hz, 6 ~ 125Hz, 7 ~ 63Hz)
  GTCCR = 1 << PWM1B | 2 << COM1B0; // General Control Register for Timer 1 - Enable use of pin OC1B, None inverting mode.

  power_usi_disable(); // Power Register - Shuts down the USI
  set_sleep_mode(SLEEP_MODE_IDLE); // Configure attiny85 sleep mode
  
  analogWrite(0, 117); //ATTiny85 Pin 5 // OC0A
  analogWrite(1, 137); //ATTiny85 Pin 6 // OC1B
  digitalWrite(3, HIGH); //ATTiny Pin 2
  delay(2000);
  digitalWrite(3, LOW);  // Flash onboard LED to show circuit is working.
}

void loop() {
  measurement = analogRead(A1); // Read battery voltage
  stepSize = abs(setPoint - measurement); // Calculate difference from set point

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
  analogWrite(4, pulseWidth); // PWM DRIVER
  analogWrite(3, (255 - pulseWidth)); // PWM LED

  // Sleep ATTiny to save power.
  wdt_enable(WDTO_15MS); // prescale of 8 ~= 15msec
  sleep_mode();          // Make CPU sleep until next WDT interrupt
  wdt_disable();
}

// Watchdog Timer Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  wdt_int(); // Needed each time
}
