#!/bin/bash

#modprobe virtio-pci
#modprobe virtio-net

echo 1 > /sys/bus/pci/devices/0000\:c1\:00.0/remove
echo 1 > /sys/bus/pci/devices/0000\:c2\:00.0/remove
echo 1 > /sys/bus/pci/devices/0000\:c3\:00.0/remove
echo 1 > /sys/bus/pci/devices/0000\:c4\:00.0/remove
echo 1 > /sys/bus/pci/devices/0000\:c5\:00.0/remove
echo 1 > /sys/bus/pci/rescan

