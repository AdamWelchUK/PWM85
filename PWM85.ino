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
*/

#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#include "PWM85.h"

const int setPoint = floatVoltage * R2 / (R2 + R1) * 1024 / regulatorVoltage;
int measurement = 0;
int pulseWidth = 0;
int stepSize = 0;

void setup() {

  pinMode(PMP1_Pin, OUTPUT); // MOSI - PUMP1
  pinMode(PMP2_Pin, OUTPUT); // MISO - PUMP2
  pinMode(VBAT_Pin, INPUT);  // SCK  - MES
  pinMode(LED_Pin, OUTPUT);  // LED
  pinMode(DRV_Pin, OUTPUT);  // DRIVER

  power_all_disable(); // Power Register - Shuts down the USI
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Configure attiny85 sleep mode

  WDT_Sleep_2S();
}

void loop() {

  PWR_start_ADC();
  measurement = analogRead(A1); // Read battery voltage
  PWR_stop_ADC();

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

  if (pulseWidth == 0)
  {
    digitalWrite(DRV_Pin, LOW);
    digitalWrite(LED_Pin, HIGH);
    PWM_stop_Driver();
    PWM_stop_Pump();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  }
  else if (pulseWidth == 255)
  {
    PWM_start_Pump();
    digitalWrite(DRV_Pin, HIGH);
    digitalWrite(LED_Pin, LOW);
    PWM_stop_Driver();
    set_sleep_mode(SLEEP_MODE_IDLE);
  }
  else
  {
    PWM_start_Pump();
    PWM_start_Driver();
    OCR1B = pulseWidth; // One counter for both outputs
    set_sleep_mode(SLEEP_MODE_IDLE);
  }

  WDT_Sleep_15MS();
}
