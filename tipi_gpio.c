#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Splett / jedimatt42.com");
MODULE_DESCRIPTION("TI-99/4A TIPI GPIO");

/* Variables for device and device class */
static dev_t tipi_device_nr;
static struct class *tipi_class;
static struct cdev control_device;
static struct cdev data_device;
static struct cdev reset_device;

#define DRIVER_NAME "tipi_gpio"
#define DRIVER_CLASS "TIPI"

/* TIPI communication pins */
#define PIN_R_RT 13
#define PIN_R_CD 21
#define PIN_R_CLK 6
#define PIN_R_DOUT 16
#define PIN_R_DIN 20
#define PIN_R_LE 19

/* TIPI Watchdog reset signal pin */
#define PIN_RESET 26

/* Register select values */
#define SEL_RC 0
#define SEL_RD 1
#define SEL_TC 2
#define SEL_TD 3

#define DEV_NAME_CONTROL "tipi_control"
#define DEV_NAME_DATA "tipi_data"
#define DEV_NAME_RESET "tipi_reset"

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
 *  Reading a byte from /dev/tipi_reset will get status of reset signal
 *
 *  None of this is implemented yet.
 */

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
  int to_copy, not_copied, delta;
  char tmp[3] = " \n";

  /* Get amount of data to copy */
  to_copy = min(count, sizeof(tmp));

  /* Read value of button */
  printk("Value of button: %d\n", gpio_get_value(17));
  tmp[0] = gpio_get_value(17) + '0';

  /* Copy data to user */
  not_copied = copy_to_user(user_buffer, &tmp, to_copy);

  /* Calculate data */
  delta = to_copy - not_copied;

  return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
/*
  int to_copy, not_copied, delta;
  char value;

  // Get amount of data to copy
  to_copy = min(count, sizeof(value));

  // Copy data from user
  not_copied = copy_from_user(&value, user_buffer, to_copy);

  // Setting the LED
  switch(value) {
    case '0':
      gpio_set_value(4, 0);
      break;
    case '1':
      gpio_set_value(4, 1);
      break;
    default:
      printk("Invalid Input!\n");
      break;
  }

  // Calculate data 
  delta = to_copy - not_copied;

  return delta; 
*/
  return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance) {
  printk("tipi_gpio - open was called!\n");
  return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
  printk("tipi_gpio - close was called!\n");
  return 0;
}

static struct file_operations control_fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close,
  .read = driver_read,
  .write = driver_write
};

static struct file_operations data_fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close
};

static struct file_operations reset_fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
  printk("Hello, Kernel!\n");

  // Allocate a device nr 
  if( alloc_chrdev_region(&tipi_device_nr, 0, 3, DRIVER_NAME) < 0) {
    printk("Device number for tipi_gpio could not be allocated!\n");
    return -1;
  }
  printk("registerd tipi_gpio Major: %d, Minor: %d - %d\n", tipi_device_nr >> 20, 
		  tipi_device_nr && 0xfffff,
		  (tipi_device_nr && 0xfffff) + 2);

  // Create device class
  if((tipi_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
    printk("tipi_gpio class can not be created!\n");
    goto CleanupDevices;
  }

  // -- Create /dev/ files

  // create device file /dev/tipi_control
  if(device_create(tipi_class, NULL, tipi_device_nr, NULL, DEV_NAME_CONTROL) == NULL) {
    printk("Can not create device file /dev/%s\n", DEV_NAME_CONTROL);
    goto CleanupClass;
  }

  // Initialize device file operations /dev/tipi_control
  cdev_init(&control_device, &control_fops);

  // Registering /dev/tipi_control to kernel
  if(cdev_add(&control_device, tipi_device_nr, 1) == -1) {
    printk("Registering of device to kernel failed!\n");
    goto CleanupFile0;
  }

  // create device file /dev/tipi_data
  if(device_create(tipi_class, NULL, tipi_device_nr + 1, NULL, DEV_NAME_DATA) == NULL) {
    printk("Can not create device file /dev/%s\n", DEV_NAME_DATA);
    goto CleanupFile0;
  }

  // Initialize device file operations /dev/tipi_data
  cdev_init(&data_device, &data_fops);

  // Registering /dev/tipi_data to kernel
  if(cdev_add(&data_device, tipi_device_nr + 1, 1) == -1) {
    printk("Registering of device to kernel failed!\n");
    goto CleanupFile1;
  }

  // create device file /dev/tipi_reset
  if(device_create(tipi_class, NULL, tipi_device_nr + 2, NULL, DEV_NAME_RESET) == NULL) {
    printk("Can not create device file /dev/%s\n", DEV_NAME_RESET);
    goto CleanupFile1;
  }

  // Initialize device file operations /dev/tipi_reset
  cdev_init(&reset_device, &reset_fops);

  // Registering /dev/tipi_reset to kernel
  if(cdev_add(&reset_device, tipi_device_nr + 2, 1) == -1) {
    printk("Registering of device to kernel failed!\n");
    goto CleanupFile2;
  }

  // Initialize GPIO access

  // PIN_R_LE init
  if(gpio_request(PIN_R_LE, "tipi-r-le")) {
    printk("Can not allocate PIN_R_LE\n");
    goto CleanupFile2;
  }

  // PIN_R_LE direction
  if(gpio_direction_output(PIN_R_LE, 0)) {
    printk("Can not set PIN_R_LE to output!\n");
    goto CleanupLE;
  }

  // PIN_R_DIN init
  if(gpio_request(PIN_R_DIN, "tipi-r-din")) {
    printk("Can not allocate GPIO 17\n");
    goto CleanupLE;
  }

  // PIN_R_DIN direction
  if(gpio_direction_input(PIN_R_DIN)) {
    printk("Can not set PIN_R_DIN to input!\n");
    goto CleanupDIN;
  }

  // PIN_R_DOUT init
  if(gpio_request(PIN_R_DOUT, "tipi-r-dout")) {
    printk("Can not allocate PIN_R_DOUT\n");
    goto CleanupDIN;
  }

  // PIN_R_DOUT direction
  if(gpio_direction_output(PIN_R_DOUT, 0)) {
    printk("Can not set PIN_R_DOUT to output!\n");
    goto CleanupDOUT;
  }

  // PIN_R_CLK init
  if(gpio_request(PIN_R_CLK, "tipi-r-clk")) {
    printk("Can not allocate PIN_R_CLK\n");
    goto CleanupDOUT;
  }

  // PIN_R_CLK direction
  if(gpio_direction_output(PIN_R_CLK, 0)) {
    printk("Can not set PIN_R_CLK to output!\n");
    goto CleanupCLK;
  }

  // PIN_R_CD init
  if(gpio_request(PIN_R_CD, "tipi-r-cd")) {
    printk("Can not allocate PIN_R_CD\n");
    goto CleanupCLK;
  }

  // PIN_R_CD direction
  if(gpio_direction_output(PIN_R_CD, 0)) {
    printk("Can not set PIN_R_CD to output!\n");
    goto CleanupCD;
  }

  // PIN_R_RT init
  if(gpio_request(PIN_R_RT, "tipi-r-rt")) {
    printk("Can not allocate PIN_R_RT\n");
    goto CleanupCD;
  }

  // PIN_R_RT direction
  if(gpio_direction_output(PIN_R_RT, 0)) {
    printk("Can not set PIN_R_RT to output!\n");
    goto CleanupRT;
  }

  /* success */
  return 0;

CleanupRT:
  gpio_free(PIN_R_RT);

CleanupCD:
  gpio_free(PIN_R_CD);

CleanupCLK:
  gpio_free(PIN_R_CLK);

CleanupDOUT:
  gpio_free(PIN_R_DOUT);

CleanupDIN:
  gpio_free(PIN_R_DIN);

CleanupLE:
  gpio_free(PIN_R_LE);

CleanupFile2:
  device_destroy(tipi_class, tipi_device_nr + 2);

CleanupFile1:
  device_destroy(tipi_class, tipi_device_nr + 1);

CleanupFile0:
  device_destroy(tipi_class, tipi_device_nr);

CleanupClass:
  class_destroy(tipi_class);

CleanupDevices:
  unregister_chrdev_region(tipi_device_nr, 3);
  return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
  gpio_set_value(PIN_R_RT, 0);
  gpio_set_value(PIN_R_CD, 0);
  gpio_set_value(PIN_R_CLK, 0);
  gpio_set_value(PIN_R_DOUT, 0);
  gpio_set_value(PIN_R_LE, 0);

  gpio_free(PIN_R_RT);
  gpio_free(PIN_R_CD);
  gpio_free(PIN_R_CLK);
  gpio_free(PIN_R_DOUT);
  gpio_free(PIN_R_DIN);
  gpio_free(PIN_R_LE);

  cdev_del(&reset_device);
  cdev_del(&data_device);
  cdev_del(&control_device);
  device_destroy(tipi_class, tipi_device_nr + 2);
  device_destroy(tipi_class, tipi_device_nr + 1);
  device_destroy(tipi_class, tipi_device_nr);
  class_destroy(tipi_class);
  unregister_chrdev_region(tipi_device_nr, 3);
  printk("tipi_gpio cleaned up\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

