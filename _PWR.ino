
inline void PWR_start_ADC()
{
  power_adc_enable();
  ADCSRA |= _BV(ADEN); // ADC on
}

inline void PWR_stop_ADC()
{
  ADCSRA &= ~_BV(ADEN); // ADC off
  power_adc_disable();
}
