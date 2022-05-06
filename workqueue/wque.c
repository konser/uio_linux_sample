#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

static void wque_work_handler(struct work_struct *w);

static struct workqueue_struct *wq = 0;
static DECLARE_DELAYED_WORK(wque_work, wque_work_handler);
static unsigned long onesec;

static void wque_work_handler(struct work_struct *w)
{
        pr_info("wque work %u jiffies\n", (unsigned)onesec);
}

static int __init wque_init(void)
{
        onesec = msecs_to_jiffies(1000);
        pr_info("wque loaded %u jiffies\n", (unsigned)onesec);

        if (!wq)
                wq = create_singlethread_workqueue("wque");
        if (wq)
                queue_delayed_work(wq, &wque_work, onesec);

        return 0;
}

static void __exit wque_exit(void)
{
        if (wq)
                destroy_workqueue(wq);
        pr_info("wque exit\n");
}

module_init(wque_init);
module_exit(wque_exit);

MODULE_DESCRIPTION("wque");
MODULE_LICENSE("GPL");
