#include<linux/init.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/slab.h>
#include<linux/string.h>
#include<linux/uaccess.h>
#include<linux/time.h>

// Part 2 is done. It just needs to be properly renamed.

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("John Thomas's individual attempt at part 2\n");

#define ENTRY_NAME "timed"			// proc name
#define PERMS 0644				// proc permissions
#define PARENT NULL				// proc parent directory
static struct file_operations fops;		// points to proc file definitions

static int first_time;
static char * message;				// message to display in proc
static int read_p;
static struct timespec clock;
static struct timespec clock2;


int hello_proc_open(struct inode *sp_inode, struct file *sp_file){
	printk(KERN_INFO "proc called open\n");

	read_p = 1;
	message = kmalloc(sizeof(char)*200, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if(message == NULL){
		printk(KERN_WARNING "hello_proc_open");
		return -ENOMEM;
	}
	if(first_time == 1){
		clock2 = clock; 	// get the time that's currently stored in proc
	}
	clock = current_kernel_time();
	sprintf(message, "Current time: %ld.%ld\n", clock.tv_sec,clock.tv_nsec);
	if(first_time == 1){
		// append the elapsed time to message
		clock2.tv_sec = clock.tv_sec - clock2.tv_sec;
		clock2.tv_nsec = clock.tv_nsec - clock2.tv_nsec;
		if(clock2.tv_nsec < 0)
		{
			clock2.tv_sec--;
			clock2.tv_nsec = 1000000000 + clock2.tv_nsec;
		}
		sprintf(message, "%sElapsed time: %ld.%ld\n",message, clock2.tv_sec, clock2.tv_nsec);
	}
	if(first_time == 0){first_time = 1;}
	return 0;
}

ssize_t hello_proc_read(struct file *sp_file, char __user * buf, size_t size, loff_t *offset){
	int len = strlen(message);

	read_p = !read_p;
	if(read_p){return 0;}

	printk(KERN_INFO "proc called read.\n");
	copy_to_user(buf, message, len);
	return len;
}

int hello_proc_release(struct inode *sp_inode, struct file *sp_file){
	printk(KERN_INFO "proc called release.\n");
	kfree(message);
	return 0;
}

static int hello_init(void){
	//printk(KERN_ALERT "Loaded John Thomas's part 2 module.\n");
	first_time = 0;
	printk(KERN_NOTICE "/proc/%s create\n",ENTRY_NAME);
	fops.open = hello_proc_open;					//
	fops.read = hello_proc_read;					// setup proc calls
	fops.release = hello_proc_release;				//

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){		//
		printk("ERROR! proc_create\n");				//
		remove_proc_entry(ENTRY_NAME, NULL);			//  Make proc entry
		return -ENOMEM;						//
	}								//

	return 0;
}
static void hello_exit(void){
	//printk(KERN_ALERT "Unloaded John Thomas's part 2 module.\n");
	remove_proc_entry(ENTRY_NAME,NULL);				// remove proc entry
	printk(KERN_NOTICE "Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(hello_init);
module_exit(hello_exit);
