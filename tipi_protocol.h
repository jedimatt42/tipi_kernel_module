/* TIPI communication pins */
/* Raspberry PI numbers */
#define PIN_CLK tipi_clk_gpio_desc

#define PIN_NIBRST tipi_nibrst_gpio_desc

#define PIN_NIB0 tipi_nib0_gpio_desc
#define PIN_NIB1 tipi_nib1_gpio_desc
#define PIN_NIB2 tipi_nib2_gpio_desc
#define PIN_NIB3 tipi_nib3_gpio_desc

/* Register select values */
#define SEL_RC 3
#define SEL_RD 2
#define SEL_TC 1
#define SEL_TD 0


/*
 *  Implements getTC, getTD, setRD, setRC as needed by tipi/services/libtipi_pibus/tipiports.c
 *
 *  Writing a byte to /dev/tipi_control will perform setRC
 *  Writing a byte to /dev/tipi_data will perform setRD
 *
 *  Reading a byte from /dev/tipi_control will perform getTC
 *  Reading a byte from /dev/tipi_data will perform getTD
 */

// --------------- TIPI io ----------------------------

// volatile to force slow memory access.
volatile long delmem = 55;

inline void signalDelay(void) {
  int i = 0;
  // sig_delay comes from kernel module parameter in tipi_pibus.c
  for(i = 0; i < sig_delay; i++) {
    delmem *= i;
  }
}

inline void regSelect(int reg) {
  // set nib0-nib3 to output mode
  gpiod_direction_output(PIN_NIB0, 1);
  gpiod_direction_output(PIN_NIB1, 1);
  gpiod_direction_output(PIN_NIB2, 1);
  gpiod_direction_output(PIN_NIB3, 1);

  // set nibrst HIGH, delay, then LOW
  gpiod_set_value(PIN_NIBRST, 1);
  signalDelay();
  gpiod_set_value(PIN_NIBRST, 0);

  // set nib0-nib3 to value of reg
  gpiod_set_value(PIN_NIB0, reg & 0x01);
  gpiod_set_value(PIN_NIB1, reg & 0x02);
  gpiod_set_value(PIN_NIB2, reg & 0x04);
  gpiod_set_value(PIN_NIB3, reg & 0x08);
  signalDelay();
  
  // now clk the data into the register
  gpiod_set_value(PIN_CLK, 1);
  signalDelay();
  gpiod_set_value(PIN_CLK, 0);
}

static unsigned char readReg(int reg) {
  unsigned char value = 0;
  // select the register to read from
  regSelect(reg);

  // set nib0-nib3 to input mode
  gpiod_direction_input(PIN_NIB0);
  gpiod_direction_input(PIN_NIB1);
  gpiod_direction_input(PIN_NIB2);
  gpiod_direction_input(PIN_NIB3);

  // after register select, the high nibble should be ready
  signalDelay();
  signalDelay();
  signalDelay();

  // read the data from the nibbles
  value |= gpiod_get_value(PIN_NIB0) << 4;
  value |= gpiod_get_value(PIN_NIB1) << 5;
  value |= gpiod_get_value(PIN_NIB2) << 6;
  value |= gpiod_get_value(PIN_NIB3) << 7;

  // now clock the low 4 bits into the PI
  gpiod_set_value(PIN_CLK, 1);
  signalDelay();
  gpiod_set_value(PIN_CLK, 0);

  // give the TI time to get the data ready
  signalDelay();

  // read the data from the nibbles
  value |= gpiod_get_value(PIN_NIB0);
  value |= gpiod_get_value(PIN_NIB1) << 1;
  value |= gpiod_get_value(PIN_NIB2) << 2;
  value |= gpiod_get_value(PIN_NIB3) << 3;

  return value;
}

static void writeReg(unsigned char value, int reg) {
  // select the register to write to
  regSelect(reg);

  // we are already in output mode
  // so now set nib0-nib3 to low 4 bits of value
  gpiod_set_value(PIN_NIB0, value & 0x10); 
  gpiod_set_value(PIN_NIB1, value & 0x20);
  gpiod_set_value(PIN_NIB2, value & 0x40);
  gpiod_set_value(PIN_NIB3, value & 0x80);
  signalDelay();

  // now clk the data into the register
  gpiod_set_value(PIN_CLK, 1);
  signalDelay();
  gpiod_set_value(PIN_CLK, 0);

  // now set nib0-nib3 to high 4 bits of value
  gpiod_set_value(PIN_NIB0, value & 0x01);
  gpiod_set_value(PIN_NIB1, value & 0x02);
  gpiod_set_value(PIN_NIB2, value & 0x04);
  gpiod_set_value(PIN_NIB3, value & 0x08);
  signalDelay();

  // now clk that data into the register
  gpiod_set_value(PIN_CLK, 1);
  signalDelay();
  gpiod_set_value(PIN_CLK, 0);
}


