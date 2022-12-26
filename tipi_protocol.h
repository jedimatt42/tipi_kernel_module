

/* TIPI communication pins */
/* Raspberry PI numbers */
#define PIN_R_RT 13
#define PIN_R_CD 21
#define PIN_R_CLK 6
#define PIN_R_DOUT 16
#define PIN_R_DIN 20
#define PIN_R_LE 19

/* Register select values */
#define SEL_RC 0
#define SEL_RD 1
#define SEL_TC 2
#define SEL_TD 3


/*
 * Plan: 
 *  Implement getTC, getTD, setRD, setRC as needed by tipi/services/libtipi_gpio/tipiports.c
 *
 *  Writing a byte to /dev/tipi_control will perform setRC
 *  Writing a byte to /dev/tipi_data will perform setRD
 *
 *  Reading a byte from /dev/tipi_control will perform getTC
 *  Reading a byte from /dev/tipi_data will perform getTD
 *
 *  None of this is implemented yet.
 */

// --------------- TIPI io ----------------------------

// volatile to force slow memory access.
volatile long delmem = 55;

int delayloop = 50;

inline void signalDelay(void) {
  int i = 0;
  for(i = 0; i < delayloop; i++) {
    delmem *= i;
  }
}

inline void regSelect(int reg) {
  gpiod_set_value(PIN_R_RT, reg & 0x02);
  gpiod_set_value(PIN_R_CD, reg & 0x01);
  signalDelay();
}

inline unsigned char parity(unsigned char input) {
  unsigned char piParity = input;
  piParity ^= piParity >> 4;
  piParity ^= piParity >> 2;
  piParity ^= piParity >> 1;
  return piParity & 0x01;
}

static unsigned char readReg(int reg) {
  int i = 7;
  unsigned char value = 0;
  int ok = 0;
  while(! ok) { // retry until parity matches
    value = 0;
    regSelect(reg);
    gpiod_set_value(PIN_R_LE, 1);
    signalDelay();
    gpiod_set_value(PIN_R_CLK, 1);
    signalDelay();
    gpiod_set_value(PIN_R_CLK, 0);
    signalDelay();
    gpiod_set_value(PIN_R_LE, 0);
    signalDelay();

    for (i=7; i>=0; i--) {
      gpiod_set_value(PIN_R_CLK, 1);
      signalDelay();
      gpiod_set_value(PIN_R_CLK, 0);
      signalDelay();
      value |= gpiod_get_value(PIN_R_DIN) << i;
    }

    // read the parity bit
    gpiod_set_value(PIN_R_CLK, 1);
    signalDelay();
    gpiod_set_value(PIN_R_CLK, 0);
    signalDelay();
    ok = gpiod_get_value(PIN_R_DIN) == parity(value);
  }
  return value;
}

static void writeReg(unsigned char value, int reg) {
  int i=7;
  int ok = 0;
  while (!ok) {
    regSelect(reg);
    for(i=7; i>=0; i--) {
      gpiod_set_value(PIN_R_DOUT, (value >>i) & 0x01);
      signalDelay();
      gpiod_set_value(PIN_R_CLK, 1);
      signalDelay();
      gpiod_set_value(PIN_R_CLK, 0);
      signalDelay();
    }

    // read the parity bit
    ok = gpiod_get_value(PIN_R_DIN) == parity(value);
  }

  gpiod_set_value(PIN_R_LE, 1);
  signalDelay();
  gpiod_set_value(PIN_R_CLK, 1);
  signalDelay();
  gpiod_set_value(PIN_R_CLK, 0);
  signalDelay();
  gpiod_set_value(PIN_R_LE, 0);
  signalDelay();
}


