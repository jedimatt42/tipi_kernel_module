#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/property.h>
#include <linux/mod_devicetable.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Splett / jedimatt42.com");
MODULE_DESCRIPTION("TI-99/4A TIPI GPIO");

/* device tree driver callbacks */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);

static struct of_device_id tipi_device_tree_ids[] = {
  {
    .compatible = "jedimatt42,tipi"
  },
  { }
};
MODULE_DEVICE_TABLE(of, tipi_device_tree_ids);

static struct platform_driver dt_driver = {
  .probe = dt_probe,
  .remove = dt_remove,
  .driver = {
    .name = "tipi_driver",
    .of_match_table = tipi_device_tree_ids
  }
};

/* GPIO */
static struct gpio_desc* tipi_cd_gpio_desc = NULL;

/* On init of device tree driver */
static int dt_probe(struct platform_device *pdev) {
  struct device *dev = &pdev->dev;

  printk("dt_probe - configuring gpio for driver...\n");

  if (!device_property_present(dev, "tipi-cd-gpio")) {
    printk("dt_probe - Error! Device property 'tipi-cd-gpio' not found!\n");
    return -1;
  }

  /* read from device properties */
  tipi_cd_gpio_desc = gpiod_get(dev, "tipi-cd" /* -gpio suffix assumed */, GPIOD_OUT_LOW);
  if (IS_ERR(tipi_cd_gpio_desc)) {
    printk("dt_probe - Error! Could not get 'tipi-cd-gpio'\n");
    return -1;
  }

  return 0;
}

/* On device tree driver cleanup */
static int dt_remove(struct platform_device *pdev) {
  gpiod_put(tipi_cd_gpio_desc);
  printk("dt_remove - removing driver\n");
  return 0;
}

/* Variables for device /dev/tipi_control /dev/tipi_data and device class */
static struct class *tipi_class;
static dev_t tipi_control_nr;
static struct cdev control_device;
static dev_t tipi_data_nr;
static struct cdev data_device;

#define DRIVER_NAME "tipi_gpio"
#define DRIVER_CLASS "TIPI"

#define DEV_REGION_SIZE 2
#define DEV_NAME_CONTROL "tipi_control"
#define DEV_NAME_DATA "tipi_data"


// --------------- being a kernel module ------------------------------

/**
 * @brief Read from the TC (TI Control) register
 */
static ssize_t driver_tc_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
  return 0;
}

/**
 * @brief Write to the RC (RPi Control) register
 */
static ssize_t driver_rc_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
  return 0;
}

/**
 * @brief Read from the TD (TI Data) register
 */
static ssize_t driver_td_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
  return 0;
}

/**
 * @brief Write to the RD (RPi Data) register
 */
static ssize_t driver_rd_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
  return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance) {
  return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
  return 0;
}

static struct file_operations control_fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close,
  .read = driver_tc_read,
  .write = driver_rc_write
};

static struct file_operations data_fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close,
  .read = driver_td_read,
  .write = driver_rd_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
  // register device_tree driver
  if (platform_driver_register(&dt_driver)) {
    printk("ModuleInit - Error! could not load driver\n");
    return -1;
  }
  printk("dt_driver - registered\n");

  // Allocate a device nr 
  if( alloc_chrdev_region(&tipi_control_nr, 0, DEV_REGION_SIZE, DRIVER_NAME) < 0) {
    printk("Device number for tipi_gpio could not be allocated!\n");
    goto CleanupDtDriver;
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

  /* success */
  return 0;

CleanupFile1:
  device_destroy(tipi_class, tipi_data_nr);

CleanupFile0:
  device_destroy(tipi_class, tipi_control_nr);

CleanupClass:
  class_destroy(tipi_class);

CleanupDevices:
  unregister_chrdev_region(tipi_control_nr /* the first one */, DEV_REGION_SIZE);

CleanupDtDriver:
  platform_driver_unregister(&dt_driver);
  return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
  cdev_del(&data_device);
  cdev_del(&control_device);
  device_destroy(tipi_class, tipi_data_nr);
  device_destroy(tipi_class, tipi_control_nr);
  class_destroy(tipi_class);
  unregister_chrdev_region(tipi_control_nr /* the first one */, DEV_REGION_SIZE);
  platform_driver_unregister(&dt_driver);
  printk("tipi_gpio cleaned up\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

