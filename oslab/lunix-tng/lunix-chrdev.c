/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * Panagiotis Xanthopoulos
 * Mirsini Kellari
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

/*
 * Global data
 */
struct cdev lunix_chrdev_cdev;

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;		// sensor that the state struct points to
	enum lunix_msr_enum type;				// state type (battery, temperature, light)
	
	WARN_ON ( !(sensor = state->sensor));
	
	// retrieve sensor type from state in order to access apropriate measurement

	type = state->type;

	// if the timestamps are not equal, then we need to update (func returns 1)

	return !(state->buf_timestamp == sensor->msr_data[type]->last_update);
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;		// sensor that the state struct points to
	enum lunix_msr_enum type;				// state type (battery, temperature, light)
	uint32_t new_msr;						// new measurement (if one is available)
	long int value;							// value returned by lookup tables
	unsigned long flags;					//flags for irqsave
	
	debug("entering state update\n");

	sensor = state->sensor;
	type = state->type;

	// below is a critical part that is protected by a spinlock
	// the msr_data of the sensor struct is updated by the protocol / sensor update functions
	// so we need to make sure that while we read the data, it wont be updated at the same time
	// creating a race condition
	// also we use irqsave because we cannot be interrupted by an interrupt
	// (that interrupt might execute code protected by the spinlock, which we cant release
	// because we were interrupted while holding the lock)
	// below code cannot sleep because it holds a spinlock and has disabled interrupts
	// (sleeping might cause a deadlock if another process tries to lock the spinlock)
	// why spinlock: sensor data must be protected from concurrent access
	// since it is updated from code executed at interrupt context,
	// we cant use semaphores because code executed at interrupt context
	// should never sleep

	spin_lock_irqsave(&sensor->lock, flags);
	
	// check if we have new measurements
	// if yes, store the new measurement in new_msr
	// and update timestamp
	// if not, free spinlock and return EAGAIN (try again)
	
	if (lunix_chrdev_state_needs_refresh(state)) {
		new_msr = sensor->msr_data[type]->values[0];
		state->buf_timestamp = sensor->msr_data[type]->last_update;
	} else {
		spin_unlock_irqrestore(&sensor->lock, flags);
		debug("no new data available");
		return -EAGAIN;
	}

	// release the spinlock

	spin_unlock_irqrestore(&sensor->lock, flags);

	// retrieve the appropriate value from lookup tables based on the state type

	switch(type) {
		case BATT:
			value = lookup_voltage[new_msr];
			break;
		case TEMP:
			value = lookup_temperature[new_msr];
			break;
		case LIGHT:
			value = lookup_light[new_msr];
		    break;
		default:
            value = 0;
            break;
	}

	state->buf_lim = snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ, "%ld.%03ld  ", value/1000, (value%1000)<0 ? -(value%1000): (value%1000));
	while(1) {
        unsigned char c = state->buf_data[state->buf_lim-3];
        if(c == '0') {
            state->buf_data[state->buf_lim-3] = ' ';
            state->buf_lim --;
        } else if(c == '.') {
			state->buf_data[state->buf_lim-3] =' ';
			state->buf_lim --;
            break;
		}
        else {
            break;
        }
    }

	// we store data in fixed 10 byte lenght
	// 1st byte is space or minus sign
	// 2nd to 4th bytes are the integer part (with zeroes for fixed length)
	// 5th byte is the dot
	// 6th to 8th bytes are the decimal part (with zeroes for fixed length)
	// 9th to 10th bytes are spaces

	// reset buf_limit and overwrite buf_data

	// state->buf_lim = 0;


	// if (value < 0) {
	// 	state->buf_data[state->buf_lim++] = '-';
	// } else {
	// 	state->buf_data[state->buf_lim++] = ' ';
	// }
	
	// value = abs(value);

	// state->buf_data[state->buf_lim++] = '0' + (value / 100000);
	// state->buf_data[state->buf_lim++] = '0' + ((value % 100000) / 10000);
	// state->buf_data[state->buf_lim++] = '0' + ((value % 10000) / 1000);
	// state->buf_data[state->buf_lim++] = '.';
	// state->buf_data[state->buf_lim++] = '0' + ((value % 1000) / 100);
	// state->buf_data[state->buf_lim++] = '0' + ((value % 100) / 10);
	// state->buf_data[state->buf_lim++] = '0' + (value % 10);
	// state->buf_data[state->buf_lim++] = ' ';
	// state->buf_data[state->buf_lim++] = ' ';

	debug("leaving state update\n");
	return 0;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	uint minor;									// minor number retrieved from inode
	struct lunix_chrdev_state_struct *state;	// state structure
	int ret;						

	debug("entering open\n");
	ret = -ENODEV;

	// prohibit anything that seeks (llseek, pread, pwrite)

	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out_with_open;
	
	// allocate zeroed memory for state struct 

	state = (struct lunix_chrdev_state_struct *)kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state) {
		goto out_with_state;
	}

	// retrieve minor number from inode

	minor = iminor(inode);

	// type is minor mod 8 (based on mk-lunix-devs.sh)

	state->type = minor % 8;

	// lunix sensors is a sensor struct array of size 16 times sizeof(sensor struct)
	// we need to point to the appropriate sensor (again look at mk-lunix-devs.sh
	// and "ls -l /dev/lunix*" after "./mk-lunix-devs.sh")

	state->sensor = &lunix_sensors[minor / 8];

	// initialize semaphore that protects the state struct at one (mutex basically)

	sema_init(&state->lock, 1);

	// private data of file struct holds the actual data of the magic device file
	// in the state struct

	filp->private_data = state;
	debug("leaving open with ret = %d\n", ret);
	return ret;

out_with_open:
	debug("leaving after failed nonseekable_open with ret = %d\n", ret);
	return ret;

out_with_state:
	debug("leaving after failed mem allocation for state struct");
	return -1;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	//deallocate data allocated for the state struct

	kfree(filp->private_data);
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Why? */
	//no need for ioctl, user should just read data
	return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{
	ssize_t ret = 0;							// if nothing is read, ret value is zero

	struct lunix_sensor_struct *sensor;			// sensor that our state points to
	struct lunix_chrdev_state_struct *state;	// state that our file struct points to

	state = filp->private_data;
	WARN_ON(!state);

	sensor = state->sensor;
	WARN_ON(!sensor);

	// the data stored at the state struct can be read by many processes at the same time
	// even though a new file pointer is created every time a file is opened, 
	// this is not the case in parent - child situations 
	// (there, the file pointer is shared, and so is the state struct itself)
	// thus, we need to protect it from concurrent access
	// we use down interruptible so that it can be interrupted (say by a signal)
	// if that happens, we return immediately because we no longer hold the semaphore

	if (down_interruptible(&state->lock)) {
		debug("failed semaphore acquisition\n");
		return -ERESTARTSYS;
	}

	// f_pos is provided by the original read syscall (and is passed down to vfs_read and then
	// to our own read)
	// it is the f_pos (current offset) of the file struct filp 
	// if it is zero, that means either that this is the first read (we need new data)
	// or that f_pos was reset to zero (rewind, we need new data as well)
	// either way, get new data
	
	if (*f_pos == 0) {

		// while state_update says try again, we try again (obviously)

		while (lunix_chrdev_state_update(state) == -EAGAIN) {

			// release sema lock (no need to hold it while we sleep)

			up(&state->lock);

			// sleep and wait until needs_refresh returns true
			// we recheck the while condition in case our process was woken up 
			// without the condition becoming true
			// in case that happened because of a signal, we return immediately

			if (wait_event_interruptible(sensor->wq, lunix_chrdev_state_needs_refresh(state))) {
				debug("error in wait event, maybe signal interrupted\n");
				return -ERESTARTSYS;
			}

			// re-aqcuire the semaphore in order to check (and maybe update)
			// the data stored at state
			// prevents race condition in case of 2+ processes waiting for the same node

			if (down_interruptible(&state->lock)) {
				debug("failed semaphore acquisition\n");
				return -ERESTARTSYS;
			}
		}

		//when we exit the while loop, we hold the sema and there is new data
	}

	// if read requests more data than is available, update
	// count with the maximum available
	
	if (*f_pos + cnt > state->buf_lim) {
		cnt = state->buf_lim - *f_pos;
	}

	// copy appropriate data to user space
	// if not all data is copied to the user space
	// copy_to_user returns nonzero, so exit with EFAULT

	if (copy_to_user(usrbuf, state->buf_data + *f_pos, cnt)) {
		ret = -EFAULT;
        goto out;
	}

	// return value becomes amount of data copied to user space

	ret = cnt;

	// update f_pos based on the data copied to user space

	*f_pos += cnt;

	// rewind f_pos if it reached buf_lim (or if it passed it, 
	// which cant happen because we check that))

	if (*f_pos >= state->buf_lim) {
		*f_pos = 0;
	}

out:

	// release the lock and return result (if we reach this, ret is
	// number of bytes read)

	up(&state->lock);
	return ret;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static loff_t lunix_chrdev_llseek(struct file *filp, loff_t offset, int whence)
{
	return -EINVAL;
}

static struct file_operations lunix_chrdev_fops = 
{
    .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap,
	.llseek 		= lunix_chrdev_llseek
};

int lunix_chrdev_init(void)
{
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
	
	debug("initializing character device\n");

	// initialize cdev struct and associate our f_ops with the corresponding field in cdev
	// also update the cdev owner module

	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);
	lunix_chrdev_cdev.owner = THIS_MODULE;
	
	// get first device number based on the major number and the first minor number

	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);

	// register the range of device numbers provided with the name of the module
	// the name specifies the device, which will appear in /proc/devices and sysfs

	ret = register_chrdev_region(dev_no, lunix_minor_cnt, THIS_MODULE->name);
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}	

	// let kernel know about our new cdev and say that it answers for the given device number range
	
	ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt);
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("character device initialization completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
		
	debug("entering\n");

	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);

	// free memory given to cdev

	cdev_del(&lunix_chrdev_cdev);

	// unregister the device number region with the previous name

	unregister_chrdev_region(dev_no, lunix_minor_cnt);

	debug("leaving\n");
}
