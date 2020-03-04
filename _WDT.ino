
#define wdt_int() WDTCR |= _BV(WDIE) // | _BV(WDCE) | _BV(WDE) // WDT goes to interrupt, not reset

// Sleep ATTiny to save power.
inline void WDT_Sleep_2S()
{ digitalWrite(3, HIGH);  //ATTiny Pin 2
  wdt_enable(WDTO_2S);
  wdt_int();
  sleep_mode();
  wdt_disable();
  digitalWrite(3, LOW);  // Flash onboard LED to show circuit is working.
}

// Sleep ATTiny to save power.
inline void WDT_Sleep_15MS()
{
  wdt_enable(WDTO_15MS); // prescale of 8 ~= 15msec
  wdt_int();
  sleep_mode();          // Make CPU sleep until next WDT interrupt
  wdt_disable();
}

// Watchdog Timer Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  wdt_int(); // Needed each time
}
