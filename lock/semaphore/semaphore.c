#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
#include <linux/semaphore.h>

/* 
 * or
 * DEFINE_SEMAPHORE(my_sem);
 *
 */
struct semaphore my_sem;
static struct task_struct *thread1, * thread2;
static int counter = 0;

/* 
 * Operations
 * void down((&sem);
 * int down_interruptible(&sem);
 * int down_trylock(&sem);
 * void up(&sem);
 */

// Function executed by kernel thread
static int thread_fn1(void *unused)
{
    while (!kthread_should_stop())
    {
		/* acquire the semaphore unless interrupted */
		if (down_interruptible(&my_sem))
		    break;
		counter++;
        printk(KERN_INFO "Thread1 Running %d\n", counter);
        ssleep(2);
		up(&my_sem);
    }
    printk(KERN_INFO "Thread1 Stopping\n");
    do_exit(0);
    return 0;
}

static int thread_fn2(void *unused)
{
	//Allow the SIGKILL signal
    while (!kthread_should_stop())
    {
		/* acquire the semaphore unless interrupted */
		if (down_interruptible(&my_sem))
		    break;
		counter++;
        printk(KERN_INFO "Thread2 Running %d\n", counter);
        ssleep(2);
		up(&my_sem);
    }
    printk(KERN_INFO "Thread2 Stopping\n");
    do_exit(0);
    return 0;
}

// Module Initialization
static int __init init_thread(void)
{
	sema_init(&my_sem, 1);
    printk(KERN_INFO "Creating Thread\n");
    //Create the kernel thread with name 'mythread1'
    thread1 = kthread_run(thread_fn1, NULL, "mythread1");
    if (thread1)
        printk(KERN_INFO "Thread1 Created successfully\n");
    else
        printk(KERN_ERR "Thread1 creation failed\n");

    thread2 = kthread_create(thread_fn2, NULL, "mythread2");
    
    if((thread2)) {
        wake_up_process(thread2);
        printk(KERN_INFO "Thread2 wakeup successfully\n");
    }
    else
        printk(KERN_ERR "Thread2 creation failed\n");
    return 0;
}
// Module Exit
static void __exit cleanup_thread(void)
{
   printk(KERN_INFO "Cleaning Up\n");
   if (thread1)
   {
       kthread_stop(thread1);
       printk(KERN_INFO "Thread1 stopped");
   }
   if (thread2)
   {
       kthread_stop(thread2);
       printk(KERN_INFO "Thread2 stopped");
   }
}
MODULE_LICENSE("GPL");
module_init(init_thread);
module_exit(cleanup_thread);
