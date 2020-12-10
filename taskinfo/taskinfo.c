#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init_task.h>
#include <linux/mm_types.h>
#include <linux/list.h>
#include <linux/pid.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
#include <linux/sched.h>
#else
#include <linux/sched/clock.h>
#include <linux/sched/task.h>
#endif

#define print_info(m, task) seq_printf(m, "%-18s%-8d%-8d%-15s%-5d%-5d " \
               "0x%-8.8lx~0x%-8.8lx "       \
               "0x%-8.8lx~0x%-8.8lx "       \
               "0x%-8.8lx~0x%-8.8lx "       \
               "0x%-8.8lx "                 \
               "%-15llu%-15llu%-15llu%15llu"    \
               "\n",                        \
            task->comm,                     \
            task->pid,                      \
            task->parent->pid,              \
            get_task_state(task),           \
            task->prio,                     \
            task->policy,                   \
            get_task_mm(task->mm, start_code), get_task_mm(task->mm, end_code), \
            get_task_mm(task->mm, start_data), get_task_mm(task->mm, end_data), \
            get_task_mm(task->mm, start_brk), get_task_mm(task->mm, brk),       \
            get_task_mm(task->mm, start_stack),                                 \
            task->sched_info.run_delay, task->utime, task->stime, task->start_time  \
            )   \

#define get_task_mm(mm, field) (unsigned long)(mm ? (mm)->field : 0)

#define INVALID_PID			-1
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 3, 0)
#define DEFINE_PROC_ATTRIBUTE(name, __write)				\
	static int name##_open(struct inode *inode, struct file *file)	\
	{								\
		return single_open(file, name##_show, PDE_DATA(inode));	\
	}								\
									\
	static const struct file_operations name##_fops = {		\
		.owner		= THIS_MODULE,				\
		.open		= name##_open,				\
		.read		= seq_read,				\
		.write		= __write,				\
		.llseek		= seq_lseek,				\
		.release	= single_release,			\
	}
#else
#define DEFINE_PROC_ATTRIBUTE(name, __write)				\
	static int name##_open(struct inode *inode, struct file *file)	\
	{								\
		return single_open(file, name##_show, PDE_DATA(inode));	\
	}								\
									\
	static const struct proc_ops name##_fops = {		\
		.proc_open		= name##_open,				\
		.proc_read		= seq_read,				\
		.proc_write		= __write,				\
		.proc_lseek		= seq_lseek,				\
		.proc_release	= single_release,			\
	}
#endif
#define DEFINE_PROC_ATTRIBUTE_RW(name)					\
	static ssize_t name##_write(struct file *file,			\
				    const char __user *buf,		\
				    size_t count, loff_t *ppos)		\
	{								\
		return name##_store(PDE_DATA(file_inode(file)), buf,	\
				    count);				\
	}								\
	DEFINE_PROC_ATTRIBUTE(name, name##_write)

#define DEFINE_PROC_ATTRIBUTE_RO(name)	\
	DEFINE_PROC_ATTRIBUTE(name, NULL)

struct task_info {
    pid_t pid;
};

static struct task_info tsk_info = {
    .pid = INVALID_PID,
};

/*state list base on 3.10, 4.18 is different */
static const char *task_state_array[] = {
        "R (running)",          /*  0 */
        "S (sleeping)",         /*  1 */
        "D (disk sleep)",       /*  2 */
        "T (stopped)",          /*  4 */
        "T (tracing stop)",     /*  8 */
        "Z (zombie)",           /* 16 */
        "X (dead)"              /* 32 */
};

static inline const char *get_task_state(struct task_struct *tsk)
{
        /* get_task_state (TASK_REPORT is 111111) */
        unsigned int state = (tsk->state & TASK_REPORT) | tsk->exit_state;
        const char **p = &task_state_array[0];

        /* unzero condition*/
        while (state) {
                p++;
                state >>= 1;
        }
        return *p;
}

static int taskall_init(void)
{
    struct task_struct *task;
    struct task_struct *inittask;
    struct list_head *list;

    printk(KERN_INFO"task all enter\n");
    printk(KERN_INFO"this %d\n",current->pid);
    printk(KERN_INFO"TASK NAME       PID   PPID    STATE       "
            "NICE POLICY   CODE     DATA    BANK    STACK\n");
    inittask = &init_task; /* proces NO.1 */
    /*
     * for_each_process(for inittask) equal list_for_each + list_entry
     */
    //list_for_each(list,&inittask->tasks) {
    //    task=list_entry(list, struct task_struct, tasks);
    for_each_process(task) {
        printk(KERN_INFO"%-18s%-8d%-8d%-15s%-5d%-5d"
               "0x%-8.8lx~0x%-8.8lx  "
               "0x%-8.8lx~0x%-8.8lx  "
               "0x%-8.8lx~0x%-8.8lx  "
               "0x%-8.8lx"
               "%lld"
               "\n",
            task->comm,
            task->pid,
            task->parent->pid,
            get_task_state(task),
            task->prio,
            task->policy,
            get_task_mm(task->mm, start_code), get_task_mm(task->mm, end_code),
            get_task_mm(task->mm, start_data), get_task_mm(task->mm, end_data),
            get_task_mm(task->mm, start_brk), get_task_mm(task->mm, brk),
            get_task_mm(task->mm, start_stack),
            task->sched_info.run_delay
            );

    }
    return 0;
}

static int pid_show(struct seq_file *m, void *ptr)
{
	struct task_info *info = m->private;

	seq_printf(m, "%d\n", info->pid);

	return 0;
}

static ssize_t pid_store(void *priv, const char __user *buf, size_t count)
{
	int pid;
    struct task_info *info = priv;

	if (kstrtoint_from_user(buf, count, 0, &pid))
		return -EINVAL;

    info->pid = pid;

	return count;
}

DEFINE_PROC_ATTRIBUTE_RW(pid);

static int taskinfo_show(struct seq_file *m, void *ptr)
{
    struct task_info *info = m->private;

    struct pid *pid_st = find_get_pid(info->pid);
    /* get_pid_task call rcu_read_lock before pid_task */
    struct task_struct * ptask = get_pid_task(pid_st, PIDTYPE_PID);
    //struct task_struct * curr = pid_task(pid_st, PIDTYPE_PID);
    
    struct list_head *thread_list;
    struct task_struct * task;
    
    if( ptask == NULL){
        printk(KERN_ALERT "No process found!\n");
        return 0;
    }
    print_info(m, ptask);
    list_for_each(thread_list, &(ptask->thread_group)) {
        task = list_entry(thread_list, struct task_struct, thread_group);
        print_info(m, task);
    }
}

DEFINE_PROC_ATTRIBUTE_RO(taskinfo);

static int taskinfo_init(void)
{
    struct proc_dir_entry *parent_dir;
    struct task_info *info = &tsk_info;
    parent_dir = proc_mkdir("task_info", NULL);
    
    if (!proc_create_data("pid", 0644, parent_dir, &pid_fops, info))
        goto remove_proc;
    
    if (!proc_create_data("taskinfo", 0, parent_dir, &taskinfo_fops, info))
        goto remove_proc;
    
    return 0;
remove_proc:
    remove_proc_subtree("task_info", NULL);
}

static void taskinfo_exit(void)
{
    remove_proc_subtree("task_info", NULL);
    printk(KERN_ALERT"taskinfo exit\n");
}

module_init(taskinfo_init);
module_exit(taskinfo_exit);
MODULE_DESCRIPTION("task_struct info Module");
MODULE_ALIAS("task_struct module");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Liming Wu <19092205@suning.com>");
