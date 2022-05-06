#!/bin/sh

# $1  0: -monitor stdio;  1: --daemonize

# -serial telnet:localhost:4321,server,nowait 替换-nographic, 因此可以通过
# telnet localhost 4321访问到虚拟机的串口；否则虚拟机的串口和GDB的输入输出给冲突了, but it can't exist with -incoming tcp:0:6666 at same time

#qemu-system-x86_64 -name centos8 -machine q35,accel=kvm,usb=off \
#       -incoming tcp:0:6666 \
#       -device vfio-pci,host=0000:19:10.4,id=hostdev2,addr=0xb \
# arm run arm vm
if [[ $1 == 0 ]]; then
qemu-kvm -machine virt,gic_version=host,usb=off,accel=kvm \
        -cpu host -smp 12,sockets=6,cores=2,threads=1 -boot menu=on -m 8192 \
         -boot menu=on \
        -bios /usr/share/edk2/aarch64/QEMU_EFI-pflash.raw \
        -drive file=/home/wlm/centos8.3_arm_uefi_host.img,media=disk,format=raw,if=none,id=systemdisk \
        -device virtio-blk-pci,scsi=off,addr=0x04,drive=systemdisk,id=systemdiskvirtio,bootindex=0 \
        -netdev tap,id=enp25s0f0v0,ifname=vnet0,script=no,downscript=no,vhost=on \
        -device virtio-net-pci,netdev=enp25s0f0v0,mac=82:0a:5e:71:bd:4a \
        -vnc :21 -D /var/log/qemu-1.log -chardev file,path=/var/log/mg.log,id=char0 \
		-monitor stdio
else
# x86 run arm vm
#qemu-kvm -machine virt,gic_version=host,usb=off,accel=kvm \
/home/wlm/qemu-6.2.0/build/qemu-system-aarch64 -machine virt \
        -cpu cortex-a57 -smp 4,sockets=2,cores=2,threads=1 -boot menu=on -m 8192 \
        -boot menu=on \
        -bios /usr/share/edk2.git/aarch64/QEMU_EFI-pflash.raw \
        -drive file=/home/wlm/centos8.3_arm_uefi_host.img,media=disk,format=raw,if=none,id=systemdisk \
        -device virtio-blk-pci,scsi=off,addr=0x04,drive=systemdisk,id=systemdiskvirtio,bootindex=0 \
        -netdev tap,id=enp25s0f0v0,ifname=vnet0,script=no,downscript=no,vhost=on \
        -device virtio-net-pci,netdev=enp25s0f0v0,mac=82:0a:5e:71:bd:4a \
        -vnc :21 -D /var/log/qemu-1.log -chardev file,path=/var/log/mg.log,id=char0 \
		-monitor stdio
fi
#brctl addif virbr0 vnet0
#ip link set vnet0 up

