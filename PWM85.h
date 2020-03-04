/*
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
