// SPDX-License-Identifier: GPL-2.0
/*
 * Record average latency of pid and its threads
 *
 * Copyright (C) 2020 Liming Wu
 *
 * Authors of code:
 *
 * Liming Wu <19092205@suning.com>
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/list.h>

#define INVALID_PID			-1

struct lat_info {
    pid_t pid;
    unsigned long long lat;
};

static struct lat_info l_info = {
    .pid = INVALID_PID,
    .lat = 0,
};

static ssize_t pid_write(struct file *file, const char __user *buf,
				   size_t count, loff_t *ppos)
{
    pid_t pid;
    struct seq_file *seq = file->private_data;
    struct lat_info *info = seq->private;

	if (kstrtoint_from_user(buf, count, 0, &pid))
		return -EINVAL;

	//if (info->pid != INVALID_PID && pid != INVALID_PID)
	//	return -EPERM;

    info->pid = pid;
    info->lat = 0;

    return count;
}

static int pid_show(struct seq_file *m, void *v) {
    struct lat_info *info = m->private;
    seq_printf(m, "%d\n", info->pid);
    return 0;
}

static int pid_open(struct inode *inode, struct  file *file) {
    return single_open(file, pid_show, PDE_DATA(inode));
}

static const struct file_operations pid_fops = {
    .owner = THIS_MODULE,
    .open = pid_open,
    .read = seq_read,
    .write = pid_write,
    .llseek = seq_lseek,
    .release = single_release,
};

static unsigned long long get_tids_lat(pid_t pid) {
    unsigned long long tids_lat = 0;
    if(pid == INVALID_PID){
        printk(KERN_ALERT "No input entered!\n");
        return 0;
    }

    struct pid *pid_st = find_get_pid(pid);
    /* get_pid_task call rcu_read_lock before pid_task */
    struct task_struct * curr = get_pid_task(pid_st, PIDTYPE_PID);
    //struct task_struct * curr = pid_task(pid_st, PIDTYPE_PID);

    if( curr == NULL){
        printk(KERN_ALERT "No process found!\n");
        return 0;
    }

    struct task_struct *task;
    struct task_struct *thread_task;
    struct list_head *list;
    struct list_head *thread_list;
    int child_cnt = 0;
    int thread_cnt = 0;

    //printk(KERN_INFO "note %d %lld\n", curr->pid, curr->sched_info.run_delay);
    tids_lat += curr->sched_info.run_delay;
    /* for each child process */
    list_for_each(list, &(curr->children)) {
        task = list_entry(list, struct task_struct, sibling);
        child_cnt += 1;
        //printk(KERN_INFO "child: %-8d %lld\n", task->pid, task->sched_info.run_delay);
        tids_lat += task->sched_info.run_delay;

        /* for each threads of pid */
        list_for_each(thread_list, &(task->thread_group)) {
            thread_task = list_entry(thread_list, struct task_struct, thread_group);
            thread_cnt += 1;
            //printk(KERN_INFO "thread: %-8d %lld\n", thread_task->pid,
            //       thread_task->sched_info.run_delay);
            tids_lat += thread_task->sched_info.run_delay;
    
            /* dont need to get child threads recursively for they at same link,
             * otherwise will dead-loop */
            // get_tids_lat(task->pid);
        }
    }
    if ( child_cnt == 0) {
        /* for each threads of pid */
        list_for_each(thread_list, &(curr->thread_group)) {
            thread_task = list_entry(thread_list, struct task_struct, thread_group);
            thread_cnt += 1;
            //printk(KERN_INFO "thread: %-8d %lld\n", thread_task->pid,
            //       thread_task->sched_info.run_delay);
            tids_lat += thread_task->sched_info.run_delay;
        }
    }
    return tids_lat/(child_cnt + thread_cnt);
}

static int lat_show(struct seq_file *m, void *v) {
    struct lat_info *info = m->private;
    unsigned long long tids_lat = get_tids_lat(info->pid);
    seq_printf(m, "%llu\n", tids_lat);
    return 0;
}

static int lat_open(struct inode *inode, struct  file *file) {
    return single_open(file, lat_show, PDE_DATA(inode));
}

static const struct file_operations lat_fops = {
    .owner = THIS_MODULE,
    .open = lat_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init pid_runqlat_init(void) {
	struct proc_dir_entry *parent_dir;
	parent_dir = proc_mkdir("pid_runqlat", NULL);
    struct lat_info *info = &l_info;
	if (!proc_create_data("pid", 0644, parent_dir, &pid_fops, info))
		goto remove_proc;

	if (!proc_create_data("runqlat", 0, parent_dir, &lat_fops, info))
		goto remove_proc;
    return 0;
remove_proc:
	remove_proc_subtree("pid_runqlat", NULL);
}

static void __exit pid_runqlat_exit(void) {
	remove_proc_subtree("pid_runqlat", NULL);
}

MODULE_DESCRIPTION("Get pid and its thread's AVG runqlat Module");
module_init(pid_runqlat_init);
module_exit(pid_runqlat_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Liming Wu <19092205@sunign.com");
