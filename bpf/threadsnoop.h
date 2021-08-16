/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __GETHOSTLATENCY_H
#define __GETHOSTLATENCY_H

#define TASK_COMM_LEN	16
#define FUN_LEN	40

struct event {
	__u64 ts;
	__u32 pid;
	char comm[TASK_COMM_LEN];
    __u64 start;
	char func[FUN_LEN];
};

#endif /* __GETHOSTLATENCY_H */
