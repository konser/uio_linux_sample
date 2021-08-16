# -*- coding: utf-8 -*-

from bcc import BPF

try:
    BPF(text='int kprobe__sys_sync(void *ctx) { bpf_trace_printk("Hello, World!\\n"); return 0; }').trace_print()
except KeyboardInterrupt:
    exit()

