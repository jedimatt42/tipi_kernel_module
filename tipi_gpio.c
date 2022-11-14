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
static dev_t tipi_control_nr;
static dev_t tipi_data_nr;
static struct class *tipi_class;
static struct cdev control_device;
static struct cdev data_device;

#define DRIVER_NAME "tipi_gpio"
#define DRIVER_CLASS "TIPI"

/* TIPI communication pins */
#define PIN_R_RT 13
#define PIN_R_CD 21
#define PIN_R_CLK 6
#define PIN_R_DOUT 16
#define PIN_R_DIN 20
#define PIN_R_LE 19

static struct gpio output_gpios[] = {
  { PIN_R_RT, GPIOF_OUT_INIT_LOW, "tipi-rt" },
  { PIN_R_CD, GPIOF_OUT_INIT_LOW, "tipi-cd" },
  { PIN_R_CLK, GPIOF_OUT_INIT_LOW, "tipi-clk" },
  { PIN_R_DOUT, GPIOF_OUT_INIT_LOW, "tipi-dout" },
  { PIN_R_LE, GPIOF_OUT_INIT_LOW, "tipi-le" }
};

static struct gpio input_gpios[] = {
  { PIN_R_DIN, GPIOF_DIR_IN, "tipi-din" }
};

/* Register select values */
#define SEL_RC 0
#define SEL_RD 1
#define SEL_TC 2
#define SEL_TD 3

#define DEV_REGION_SIZE 2
#define DEV_NAME_CONTROL "tipi_control"
#define DEV_NAME_DATA "tipi_data"

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

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
  return 0;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
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

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
  printk("Hello, Kernel!\n");

  // Allocate a device nr 
  if( alloc_chrdev_region(&tipi_control_nr, 0, DEV_REGION_SIZE, DRIVER_NAME) < 0) {
    printk("Device number for tipi_gpio could not be allocated!\n");
    return -1;
  }
  tipi_data_nr = tipi_control_nr + 1;
  printk("registerd tipi_control Major: %d, Minor: %d\n", tipi_control_nr >> 20, tipi_control_nr & 0xfffff);
  printk("registerd tipi_data Major: %d, Minor: %d\n", tipi_data_nr >> 20, tipi_data_nr & 0xfffff);

  // Create device class
  if((tipi_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
    printk("tipi_gpio class can not be created!\n");
    goto CleanupDevices;
  }

  // -- Create /dev/ files

  // create device file /dev/tipi_control
  if(device_create(tipi_class, NULL, tipi_control_nr, NULL, DEV_NAME_CONTROL) == NULL) {
    printk("Can not create device file /dev/%s\n", DEV_NAME_CONTROL);
    goto CleanupClass;
  }

  // Initialize device file operations /dev/tipi_control
  cdev_init(&control_device, &control_fops);

  // Registering /dev/tipi_control to kernel
  if(cdev_add(&control_device, tipi_control_nr, 1) == -1) {
    printk("Registering of device to kernel failed!\n");
    goto CleanupFile0;
  }

  // create device file /dev/tipi_data
  if(device_create(tipi_class, NULL, tipi_data_nr, NULL, DEV_NAME_DATA) == NULL) {
    printk("Can not create device file /dev/%s\n", DEV_NAME_DATA);
    goto CleanupFile0;
  }

  // Initialize device file operations /dev/tipi_data
  cdev_init(&data_device, &data_fops);

  // Registering /dev/tipi_data to kernel
  if(cdev_add(&data_device, tipi_data_nr, 1) == -1) {
    printk("Registering of device to kernel failed!\n");
    goto CleanupFile1;
  }

  // Initialize GPIO access

  if(gpio_request_array(input_gpios, ARRAY_SIZE(input_gpios))) {
    printk("Can not allocate input pins\n");
    goto CleanupFile1;
  }

  if(gpio_request_array(output_gpios, ARRAY_SIZE(output_gpios))) {
    printk("Can not allocate input pins\n");
    goto CleanupInputGpios;
  }

  /* success */
  return 0;

CleanupInputGpios:
  gpio_free_array(input_gpios, ARRAY_SIZE(input_gpios));

CleanupFile1:
  device_destroy(tipi_class, tipi_data_nr);

CleanupFile0:
  device_destroy(tipi_class, tipi_control_nr);

CleanupClass:
  class_destroy(tipi_class);

CleanupDevices:
  unregister_chrdev_region(tipi_control_nr /* the first one */, DEV_REGION_SIZE);
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

  gpio_free_array(input_gpios, ARRAY_SIZE(input_gpios));
  gpio_free_array(output_gpios, ARRAY_SIZE(output_gpios));

  cdev_del(&data_device);
  cdev_del(&control_device);
  device_destroy(tipi_class, tipi_data_nr);
  device_destroy(tipi_class, tipi_control_nr);
  class_destroy(tipi_class);
  unregister_chrdev_region(tipi_control_nr /* the first one */, DEV_REGION_SIZE);
  printk("tipi_gpio cleaned up\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

