#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/cpumask.h>

#define err(msg) printk(KERN_ERR "%s\n", msg)

static DEFINE_PER_CPU(long, cpuvar) = 10;
static long __percpu *cpualloc;

static int percpu_init(void)
{
    long *p;
    int i;
    
    cpualloc = alloc_percpu(long);
    if (!cpualloc) {
        err("alloc_percpu");
        goto err;
    }
    
    for_each_possible_cpu(i) {
        p = per_cpu_ptr(cpualloc, i);
        *p = i;
    }
    
    for_each_possible_cpu(i) {
        p = per_cpu_ptr(cpualloc, i);
        printk(KERN_INFO "%d = %ld\n", i, *p);
    }
    
    
    for_each_possible_cpu(i) {
        per_cpu(cpuvar, i) = 18;
    }
    
    pr_info("init: cpuvar on cpu%d  = %ld\n",
                           smp_processor_id(), get_cpu_var(cpuvar)++);

    put_cpu_var(cpuvar);
    
    /* alloc a percpu value */
    cpualloc = alloc_percpu(long);
    
    /* set all cpu for this value */
    for_each_possible_cpu(i)
        *per_cpu_ptr(cpualloc, i) = 100;
    
    return 0;
err:
    return -1;
}

static void percpu_exit(void)
{
    int cpu;
    pr_info("exit module...\n");

    for_each_possible_cpu(cpu)
        pr_info("cpuvar cpu%d = %ld\n", cpu, per_cpu(cpuvar, cpu));

    pr_info("exit: cpualloc = %ld\n", *per_cpu_ptr(cpualloc, smp_processor_id()));
    free_percpu(cpualloc);

    pr_info("Bye: module unloaded from 0x%p\n", percpu_exit);
}

module_init(percpu_init);
module_exit(percpu_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Liming Wu <19092205@suning.com>");
