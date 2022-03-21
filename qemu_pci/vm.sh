#!/bin/sh

# $1  0: -monitor stdio;  1: --daemonize

# -serial telnet:localhost:4321,server,nowait 替换-nographic, 因此可以通过 
# telnet localhost 4321访问到虚拟机的串口；否则虚拟机的串口和GDB的输入输出给冲突了, but it can't exist with -incoming tcp:0:6666 at same time

#qemu-system-x86_64 -name centos8 -machine q35,accel=kvm,usb=off \
#	-incoming tcp:0:6666 \
qemu-system-x86_64 -name centos8 -machine pc-i440fx-2.1,accel=kvm \
	-cpu host -m 8192 -smp 4,sockets=2,cores=2,threads=1 -boot menu=on \
	-drive file=/home/wlm/centos8.4-8g-ext4-host.img,media=disk,format=raw,if=none,id=systemdisk \
	-device virtio-blk-pci,scsi=off,addr=0x04,drive=systemdisk,id=systemdiskvirtio,bootindex=0 \
	-netdev tap,id=ens47f0v0,ifname=vnet0,script=no,downscript=no,vhost=on \
	-device virtio-net-pci,netdev=ens47f0v0,mac=00:16:35:AF:94:4B \
	-vnc :21 -D /var/log/qemu-1.log -chardev file,path=/var/log/mg.log,id=char0 \
	-serial chardev:char0 -trace events=/home/wlm/events \
	-serial telnet:localhost:4321,server,nowait \
	-device edu  \
	-monitor stdio

brctl addif virbr0 vnet0
ip link set vnet0 up
