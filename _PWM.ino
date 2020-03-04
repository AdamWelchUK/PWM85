
// PWM Timer/Counters 0
inline void PWM_start_Pump()
{
  power_timer0_enable();
  TCCR0A = 2 << COM0A0 | 3 << COM0B0 | 3 << WGM00; // Timer 0 Control Register A -  Enable Fast PWM, OC0A Non Inverting Mode, OC0B Inverting Mode (OC0A)
  TCCR0B = 0 << WGM02 | 1 << CS00; // Timer 0 Control Register B - Enable Fast PWM, Clock Select Bit no prescaling
  OCR0A = 117; // set pwm duty // analogWrite(0, 117); //ATTiny85 Pin 5 // OC0A // PMP1_Pin
  OCR0B = 137; // set pwm duty // analogWrite(1, 137); //ATTiny85 Pin 6 // OC0B // PMP2_Pin
}

inline void PWM_stop_Pump()
{
  digitalWrite(PMP1_Pin, HIGH);
  digitalWrite(PMP2_Pin, HIGH);
  power_timer0_disable();
}

// PWM Timer/Counter 1
inline void PWM_start_Driver()
{
  power_timer1_enable();
  TCCR1 = 6 << CS10; // Timer 1 Control Register -  Set Prescaler (1<<CS10 ~ 4kHz, 2 ~ 2kHz, 3 ~ 1kHz, 4 ~ 500Hz, 5 ~ 250Hz, 6 ~ 125Hz, 7 ~ 63Hz)
  GTCCR = 1 << PWM1B | 1 << COM1B0; // General Control Register for Timer 1 - *PWM*, OC1B and ~OC1B connected. // DRV_Pin ~LED_Pin
}

inline void PWM_stop_Driver()
{
  power_timer1_disable();
}
