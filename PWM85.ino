/* PWM Solar charge controller using Julian Iletts hardware design and code (https://www.youtube.com/playlist?list=PLjzGSu1yGFjUsB3ChZJ2_-MTNp7bHaKED)
  but adapted to work on the ATTiny25/45/85 by Adam Welch (http://adamwelch.co.uk).
*/

#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

/*
  ArduinoIDE
  To program the ATTiny in the arduino IDE you will need the board definitions installing.  I suggest installing
  Spence Konde's ATTinyCore which is available in the board manager (Tools - Board - Board Manager - search for ATTinyCore)

  For the highest efficiency you should change the ATTiny to an internal 1MHz clock speed -
  to apply this setting click Burn Bootloader in the Arduino IDE.

  Once you have installed this board definition, you can disable the millis()/micros() to save power.

  This code should work on all ATTiny25/45/85s but make sure to select the right one when uploading your code.
  To get three PWM outputs on the ATTiny (and set them reliably) I found these sites super useful:
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

#define PMP1_Pin 0 //MOSI - PUMP1
#define PMP2_Pin 1 //MISO - PUMP2
#define VBAT_Pin 2 //SCK  - VBAT Mes
#define LED_Pin  3 //LED
#define DRV_Pin  4 //Driver

/* Battery Charging Set Point.

  This equation is calculated at compile time to set the interger which the charge controller uses as it's modulation set point.
  By default this is set to a value to charge the battery to 13.5 volts.  Change this number if you wish - absolute maximum 16.8v

  It is also reconmended to test the output of the regulator and use this equation to calibrate the charge controller.

  floatV = battery float voltage (13.5 volts by default)
  regulatorV = output from the voltage regulator (5.0 or 3.3 volts typically - but check yours!)
  R1 = value of R1 in the circuit (82000 ohms by design)
  R2 = value of R2 in the circuit (20000 ohms by design)
*/

#define floatV 13.5
#define regulatorV 3.3
#define R1 82000
#define R2 20000

#define wdt_int() WDTCR |= _BV(WDIE) // | _BV(WDCE) | _BV(WDE) // WDT goes to interrupt, not reset

const int setPoint = floatV * R2 / (R2 + R1) * 1024 / regulatorV;
int measurement = 0;
int stepSize = 0;
int pulseWidth = 0;

////////////////////
void setup()
{
  pinMode(PMP1_Pin, OUTPUT); // MOSI - Pump1
  pinMode(PMP2_Pin, OUTPUT); // MISO - Pump2
  pinMode(VBAT_Pin, INPUT);  // SCK  - ADC
  pinMode(LED_Pin, OUTPUT);  // LED
  pinMode(DRV_Pin, OUTPUT);  // DRIVER

  power_usi_disable(); // Power Register - Shuts down the USI
  PWR_stop_ADC(); // Stop ADC to save power

  set_sleep_mode(SLEEP_MODE_IDLE); // Configure attiny85 sleep mode

  PWM_setup_Pump();  // Configure registers for most efficient charge pump
  PWM_setup_Driver();  // Configure registers for mosfet driver pin
  OCR1B = 0;  // Write mosfet pin LOW & LED HIGH

  WDT_Sleep(WDTO_2S);
}

////////////////////
void loop()
{
  PWR_start_ADC(); // Start ADC
  measurement = analogRead(A1); // Read battery voltage
  PWR_stop_ADC(); // Stop ADC to save power

  stepSize = setPoint - measurement; // Important : Can be negative
  pulseWidth += stepSize;            // Can decrease too!
  pulseWidth = constrain(pulseWidth, 0, 255); // prevent wrap around

  OCR1B = pulseWidth; // Write pulseWidth value to both mosfet and led pins.
  WDT_Sleep(WDTO_60MS);
}

////////////////////
void PWR_start_ADC()
{
  power_adc_enable(); // Enable ADC in power register
  ADCSRA |= _BV(ADEN); // ADC on
}

////////////////////
void PWR_stop_ADC()
{
  ADCSRA &= ~_BV(ADEN); // ADC off
  power_adc_disable(); // Disable ADC in power register
}

////////////////////
void PWM_setup_Pump()
{
  TCCR0A = 2 << COM0A0 | 3 << COM0B0 | 3 << WGM00; // Timer 0 Control Register A -  Enable Fast PWM, OC0A Non Inverting Mode, OC0B Inverting Mode (OC0A)
  TCCR0B = 0 << WGM02 | 1 << CS00; // Timer 0 Control Register B - Enable Fast PWM, Clock Select Bit no prescaling
  OCR0A = 117; // set PWM duty // analogWrite(0, 117); // OC0A // PMP1_Pin
  OCR0B = 137; // set PWM duty // analogWrite(1, 137); // OC0B // PMP2_Pin
}

////////////////////
void PWM_setup_Driver()
{
  TCCR1 = 6 << CS10; // Timer 1 Control Register -  Set Prescaler (1<<CS10 ~ 4kHz, 2 ~ 2kHz, 3 ~ 1kHz, 4 ~ 500Hz, 5 ~ 250Hz, 6 ~ 125Hz, 7 ~ 63Hz)
  GTCCR = 1 << PWM1B | 1 << COM1B0; // General Control Register for Timer 1 - *PWM*, OC1B and ~OC1B connected. // DRV_Pin ~LED_Pin
}

////////////////////
void WDT_Sleep(byte WDTO_time)
{
  wdt_enable(WDTO_time); // Enable watchdog timer ~= 2sec
  wdt_int();
  sleep_mode(); // Make CPU sleep until next WDT interrupt
  wdt_disable();
}

////////////////////
// Watchdog Timer Interrupt Service is executed when watchdog timed out
ISR(WDT_vect) {
  wdt_int(); // Needed each time
}
