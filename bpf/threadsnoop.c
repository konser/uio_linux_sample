/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */

/*
 * threadsnoop   Trace thread creation via pthread_create().
 *
 * Copyright (c) 2021 Liming Wu
 *
 * Based on threadsnoop(8) from BCC by Brendan Gregg.
 * 13-Aug-2021   Liming Wu  Created this.
 */
#include <argp.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "threadsnoop.h"
#include "threadsnoop.skel.h"
#include "trace_helpers.h"
#include "uprobe_helpers.h"

#define PERF_BUFFER_PAGES	16
#define PERF_POLL_TIMEOUT_MS	100
#define warn(...) fprintf(stderr, __VA_ARGS__)

static volatile sig_atomic_t exiting = 0;
static const char *libc_path = NULL;
static struct syms_cache *syms_cache = NULL;

const char *argp_program_version = "threadsnoop 0.1";
const char *argp_program_bug_address =
	"https://github.com/iovisor/bcc/tree/master/libbpf-tools";
const char argp_program_doc[] =
" Trace thread creation via pthread_create()\n"
"\n";

static const struct argp_option opts[] = {
	{ NULL, 'h', NULL, OPTION_HIDDEN, "Show the full help" },
	{},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'h':
		argp_state_help(state, stderr, ARGP_HELP_STD_HELP);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static void sig_int(int signo)
{
	exiting = 1;
}

static void handle_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
	const struct event *event = data;
    __u32 start_ts = 0;
	const struct syms *syms;
	const struct sym *sym;
    
    if (start_ts == 0) {
        start_ts = event->ts;
    }

	syms = syms_cache__get_syms(syms_cache, event->pid);
	if (!syms) {
		fprintf(stderr, "failed to get syms\n");
    }
	sym = syms__map_addr(syms, event->pid);
	if (sym)
		printf("    %s\n", sym->name);
	else
		printf("    [unknown]\n");
    //printf("%-10lld %-6d %-16s %-16s\n", (event->ts - start_ts) / 1000000,
    //    event->pid, event->comm, sym->name);
    printf("%-10lld %-6d %-16s \n", (event->ts - start_ts) / 1000000,
        event->pid, event->comm);
}

static void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt)
{
	warn("lost %llu events on CPU #%d\n", lost_cnt, cpu);
}

static int get_libc_path(char *path)
{
	FILE *f;
	char buf[PATH_MAX] = {};
	char *filename;
	float version;

	if (libc_path) {
		memcpy(path, libc_path, strlen(libc_path));
		return 0;
	}

	f = fopen("/proc/self/maps", "r");
	if (!f)
		return -errno;

	while (fscanf(f, "%*x-%*x %*s %*s %*s %*s %[^\n]\n", buf) != EOF) {
		if (strchr(buf, '/') != buf)
			continue;
		filename = strrchr(buf, '/') + 1;
		if (sscanf(filename, "libc-%f.so", &version) == 1) {
			memcpy(path, buf, strlen(buf));
			fclose(f);
			return 0;
		}
	}

	fclose(f);
	return -1;
}

static int attach_uprobes(struct threadsnoop_bpf *obj)
{
	int err;
	off_t func_off;
    /*
	char libc_path[PATH_MAX] = {};
	err = get_libc_path(libc_path);
	if (err) {
		warn("could not find libc.so\n");
		return -1;
	}*/

	char libc_path[PATH_MAX] = "/usr/lib64/libpthread-2.31.so";
	//func_off = get_elf_func_offset(libc_path, "pthread_create");
	func_off = get_elf_func_offset("/usr/lib64/libpthread-2.31.so", "pthread_create");
	if (func_off < 0) {
		warn("could not find pthread_create in %s\n", libc_path);
		return -1;
	}
	obj->links.handle_entry =
		bpf_program__attach_uprobe(obj->progs.handle_entry, false,
					   -1, libc_path, func_off);
	err = libbpf_get_error(obj->links.handle_entry);
	if (err) {
		warn("failed to attach pthread_create: %d\n", err);
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	static const struct argp argp = {
		.options = opts,
		.parser = parse_arg,
		.doc = argp_program_doc,
	};
	struct perf_buffer_opts pb_opts;
	struct perf_buffer *pb = NULL;
	struct threadsnoop_bpf *obj;
	int err;

	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err)
		return err;

	err = bump_memlock_rlimit();
	if (err) {
		warn("failed to increase rlimit: %d\n", err);
		return 1;
	}

	obj = threadsnoop_bpf__open();
	if (!obj) {
		warn("failed to open BPF object\n");
		return 1;
	}


	err = threadsnoop_bpf__load(obj);
	if (err) {
		warn("failed to load BPF object: %d\n", err);
		goto cleanup;
	}

	syms_cache = syms_cache__new(0);
	if (!syms_cache) {
		fprintf(stderr, "failed to create syms_cache\n");
		goto cleanup;
	}
	err = attach_uprobes(obj);
	if (err)
		goto cleanup;

	pb_opts.sample_cb = handle_event;
	pb_opts.lost_cb = handle_lost_events;
	pb = perf_buffer__new(bpf_map__fd(obj->maps.events), PERF_BUFFER_PAGES,
			&pb_opts);
	err = libbpf_get_error(pb);
	if (err) {
		warn("failed to open perf buffer: %d\n", err);
		goto cleanup;
	}

	if (signal(SIGINT, sig_int) == SIG_ERR) {
		warn("can't set signal handler: %s\n", strerror(-errno));
		goto cleanup;
	}

	printf("%-8s %-7s %-16s %-10s\n",
	       "TIME(ms)", "PID", "COMM", "FUNC");

	while (1) {
		if ((err = perf_buffer__poll(pb, PERF_POLL_TIMEOUT_MS)) < 0)
			break;

		if (exiting)
			goto cleanup;
	}
	warn("error polling perf buffer: %d\n", err);

cleanup:
	perf_buffer__free(pb);
	threadsnoop_bpf__destroy(obj);
	syms_cache__free(syms_cache);

	return err != 0;
}
