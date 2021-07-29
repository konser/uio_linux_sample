#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/gfp.h>

static struct task_struct *thread_st;
struct slab_test {
   u64 test; 
};

static int num;
static struct kmem_cache *test_kmem;
static struct kmem_cache *test_kmem1;
struct kvm_mmu_page *obj1 = NULL, *obj2 = NULL;

void slab_ctor(void *cachep){
    printk("slab_ctor is called! object %d has been inited!\n", num);
    num++;
}

// Function executed by kernel thread
static int thread_fn(void *unused)
{
	//Allow the SIGKILL signal
    allow_signal(SIGKILL);
    test_kmem =  kmem_cache_create("test_kmem", sizeof(struct kvm_mmu_page), 0, 0, slab_ctor);
    test_kmem1 =  kmem_cache_create("test_kmem1", sizeof(struct kvm_mmu_page), 0, SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD, slab_ctor);

    if (!test_kmem)
            return -ENOMEM;

    if (!test_kmem1)
            return -ENOMEM;

    obj1 = kmem_cache_alloc(test_kmem, GFP_KERNEL); 
    obj2 = kmem_cache_alloc(test_kmem1, GFP_KERNEL); 

    if (!obj1)
            return -ENOMEM;
    else
            printk("slab obj1 created\n");

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
       if (obj1) {
           kmem_cache_free(test_kmem, obj1);
           printk("free obj1\n");
       }
       if(test_kmem) {
           kmem_cache_destroy(test_kmem);
           printk("destroy kmem\n");
       }
       if (obj2) {
           kmem_cache_free(test_kmem1, obj2);
           printk("free obj2\n");
       }
       if(test_kmem1) {
           kmem_cache_destroy(test_kmem1);
           printk("destroy kmem1\n");
       }
       kthread_stop(thread_st);
       printk(KERN_INFO "Thread stopped");
   }
}
MODULE_LICENSE("GPL");
module_init(init_thread);
module_exit(cleanup_thread);
