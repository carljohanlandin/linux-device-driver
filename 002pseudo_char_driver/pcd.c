#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];

/* This holds the device number */
dev_t device_number;

/* cdev variable */
struct cdev pcd_cdev;

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t temp;

	pr_info("llseek requested\n");
	pr_info("current value of the file position %lld\n", filp->f_pos);

	switch (whence)
	{
		case SEEK_SET:
			if ((offset > DEV_MEM_SIZE) || (offset < 0))
				return -EINVAL;
		
			filp->f_pos = offset;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if (temp > DEV_MEM_SIZE || temp < 0)
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = DEV_MEM_SIZE + offset;
			if (temp > DEV_MEM_SIZE || temp < 0)
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}

	pr_info("New value of the file position = %lld\n, filp->f_pos);

	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	pr_info("read requested for %zu bytes\n", count);
	pr_info("Current file position = %lld\n", *f_pos);
	/* Adjust the 'count' */
	if (*f_pos + count > DEV_MEM_SIZE) {
		count = DEV_MEM_SIZE - *f_pos;
	}

	/* copy to user */
	if (copy_to_user(buff, &device_buffer[*f_pos], count)) {
		return -EFAULT;
	}

	/* update the current file position */
	*f_pos += count;

	pr_info("Number of bytes successfully read = %zu\n", count);
	pr_info("Updated file position = &lld\n", *f_pos);

	/* return the number of bytes which have been sucessfully read */
	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	pr_info("write requested for %zu bytes\n", count);
	pr_info("Current file position = %lld\n", *f_pos);

	/* Adjust the 'count' */
	if (*f_pos + count > DEV_MEM_SIZE) {
		count = DEV_MEM_SIZE - *f_pos;
	}

	if (count == 0) {
		return -ENOMEM;
	}

	/* copy from user */
	if (copy_from_user(&device_buffer[*f_pos], buff, count)) {
		return -EFAULT;
	}

	/* update the current file position */
	*f_pos += count;

	pr_info("Number of bytes successfully written = %zu\n", count);
	pr_info("Updated file position = &lld\n", *f_pos);

	/* return the number of bytes which have been sucessfully written */
	return count;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	pr_info("open was successful\n");
	return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("release was successful\n");
	return 0;
}

/* File operations of the driver */
struct file_operations pcd_fops = {
	.open = pcd_open,
	.write = pcd_write,
	.read = pcd_read,
	.llseek = pcd_llseek,
	.release = pcd_release,
	.owner = THIS_MODULE
};

struct class *class_pcd;
struct device *device_pcd;

static int __init pcd_driver_init(void)
{
	/* 1. Dynamically allocate a device number */
	alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");

	pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

	/* 2. Initialize the cdev structure with fops */
	cdev_init(&pcd_cdev, &pcd_fops);

	/* 3. Register a device (cdev structure) with VFS */
	cdev_add(&pcd_cdev, device_number, 1);
	pcd_cdev.owner = THIS_MODULE;

	/* 4. Create device class under /sys/class/ */
	class_pcd = class_create(THIS_MODULE, "pcd_class");
	
	/* 5. Populate the sysfs with device information */
	device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");

	pr_info("Module init was successful\n");

	return 0;
}

static void __exit pcd_driver_cleanup(void)
{
	device_destroy(class_pcd, device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carl-Johan Landin");
MODULE_DESCRIPTION("A pseudo character driver");
