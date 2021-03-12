#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>

static struct task_struct *thread_st;
// Function executed by kernel thread
static int thread_fn(void *unused)
{
	//Allow the SIGKILL signal
    allow_signal(SIGKILL);
    while (!kthread_should_stop())
    {
        printk(KERN_INFO "Thread Running\n");
        ssleep(5);
		//Check if the signal is pending
		if (signal_pending(thread_st))
		    break;
    }
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}

// Module Initialization
static int __init init_thread(void)
{
    printk(KERN_INFO "Creating Thread\n");
    //Create the kernel thread with name 'mythread1'
    thread_st = kthread_run(thread_fn, NULL, "mythread1");
    if (thread_st)
        printk(KERN_INFO "Thread1 Created successfully\n");
    else
        printk(KERN_ERR "Thread1 creation failed\n");
    return 0;
}
// Module Exit
static void __exit cleanup_thread(void)
{
   printk(KERN_INFO "Cleaning Up\n");
   if (thread_st)
   {
       kthread_stop(thread_st);
       printk(KERN_INFO "Thread stopped");
   }
}
MODULE_LICENSE("GPL");
module_init(init_thread);
module_exit(cleanup_thread);
