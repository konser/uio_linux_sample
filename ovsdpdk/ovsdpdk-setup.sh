#!/bin/bash

echo "-> getting config ..."
declare -A ovs_config
conf_path="/etc/ovs-dpdk.conf"
if [ ! -f $conf_path ];then
        echo "   /etc/ovs-dpdk.conf not found"
        touch $conf_path
        cat > $conf_path << EOF
service_type=compute
vf_id=
EOF
echo "   a empty config file created,please set the config file"
#exit 1
else
        conf=$(cat $conf_path)
        for line in ${conf[@]};do
                key=$(echo $line | awk -F '=' '{print $1}')
                val=$(echo $line | awk -F '=' '{print $2}')
                if [ -z "$val" ] && [ $key != "vf_id" ] && [ $key != "phy_ip" ]; then
                        echo "  empty value for $key"
                        exit 1
                fi
                ovs_config[$key]=$val
        done
fi


echo "-> getting system info ..."
### get bus-id ###
vf_id=${ovs_config['vf_id']}
if [ -n  "$vf_id" ] ; then
        bus_id_0=$(cat /sys/class/net/service0/device/virtfn$vf_id/uevent 2>/dev/null | grep PCI_SLOT_NAME | awk -F '=' '{print $2}')
        bus_id_1=$(cat /sys/class/net/service1/device/virtfn$vf_id/uevent 2>/dev/null | grep PCI_SLOT_NAME | awk -F '=' '{print $2}')
        driver=$(cat /sys/class/net/service0/device/virtfn$vf_id/uevent 2>/dev/null | grep DRIVER | awk -F '=' '{print $2}')
else
        bus_id_0=$(cat /sys/class/net/service0/device/uevent 2>/dev/null| grep PCI_SLOT_NAME | awk -F '=' '{print $2}')
        bus_id_1=$(cat /sys/class/net/service1/device/uevent 2>/dev/null| grep PCI_SLOT_NAME | awk -F '=' '{print $2}')
        driver=$(cat /sys/class/net/service0/device/uevent 2>/dev/null| grep DRIVER | awk -F '=' '{print $2}')
fi
bus_ids="$bus_id_0 $bus_id_1"
if [ -z "$driver" ] && [ -z  "$vf_id" ] ;then
    bus_ids=''
        for p in /sys/bus/pci/devices/*/uevent; do
                cat $p | grep -q vfio-pci
                if [ $? -eq 0 ];then
                        bus_ids="$bus_ids$(echo $p | awk -F '/' '{print $6}') "
                fi
        done
        driver="vfio-pci"
fi


bus_ids=($bus_ids)
### get mac ###
if [ -n  "$vf_id" ] ; then
        mac_0=$(/usr/sbin/ip link show service0 | grep "vf $vf_id" | sed  's/,//g' | awk '{print $4}')
        mac_1=$(/usr/sbin/ip link show service0 | grep "vf $vf_id" | sed  's/,//g' | awk '{print $4}')
        [ "$mac_0" == "00:00:00:00:00:00" ] && mac_0=$(cat /sys/class/net/service0/device/virtfn$vf_id/net/*/address)
        [ "$mac_1" == "00:00:00:00:00:00" ] && mac_1=$(cat /sys/class/net/service1/device/virtfn$vf_id/net/*/address)
else
        udev_rules_0=$(cat /etc/udev/rules.d/70-persistent-net.rules  | grep service0)
        udev_rules_1=$(cat /etc/udev/rules.d/70-persistent-net.rules  | grep service1)
        mac_0=$(echo ${udev_rules_0#*'{address}=='} | awk -F ',' '{print $1}')
        mac_1=$(echo ${udev_rules_1#*'{address}=='} | awk -F ',' '{print $1}')
fi
### get cpu-mask ###
cmdline=$(cat /etc/default/grub | grep isolcpus)
isolcpus=$(echo ${cmdline#*'isolcpus='} | awk '{print $1}' | sed 's/,/ /g')
cpu_mask=0
for id in ${isolcpus[@]};do cpu_mask=$(($cpu_mask | 1 << $id));done
cpu_mask_hex=$(printf "0x%x" $cpu_mask)
### dump system info ###
for id in ${bus_ids[@]};do echo "   $id:$driver";done
[ -n  "$vf_id" ] && echo "   vf_id:$vf_id"
echo "   mac_0:$mac_0"
echo "   mac_1:$mac_1"
echo "   isolcpus:$isolcpus"

#exit 0



echo "-> binding vfio-pci driver ..."
/usr/sbin/modprobe vfio-pci
if [ "$driver" != "vfio-pci" ] && [ "$driver" != "mlx5_core" ] ;then
        for id in ${bus_ids[@]};
        do
                echo $id > "/sys/bus/pci/drivers/$driver/unbind"
                echo 'vfio-pci' > "/sys/bus/pci/devices/$id/driver_override"
                echo $id > "/sys/bus/pci/drivers/vfio-pci/bind"
                cat "/sys/bus/pci/devices/$id/uevent" | grep -q "vfio-pci"
                if [ $? -eq 0 ];then
                        echo "   $id:vfio-pci";
                else
                        echo "   $id:failed-to-bind";
                        exit 1
                fi
        done
else
        echo "   skipped"
fi


echo "-> configing openvswitch ..."
dpdk_rxq=4
sleep 1
phy_ip=${ovs_config['phy_ip']}
ovs-vsctl --no-wait set Open_vSwitch . other_config:pmd-cpu-mask=$cpu_mask_hex
echo "   pmd-cpu-mask:$cpu_mask_hex"
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-init=true
echo "   dpdk-init:true"
ovs-vsctl --may-exist add-br br-bond1 -- \
                        set Bridge br-bond1 datapath_type=netdev -- \
                        br-set-external-id br-bond1 bridge-id br-bond1  -- \
                        set bridge br-bond1 fail-mode=standalone other_config:hwaddr=$mac_0
ovs-vsctl --may-exist add-bond br-bond1 dpdkbond dpdk0 dpdk1 bond_mode=balance-tcp other_config:lb-output-action=true -- \
                        set Interface dpdk0 type=dpdk options:dpdk-devargs=${bus_ids[0]} options:n_rxq="4"  -- \
                        set Interface dpdk1 type=dpdk options:dpdk-devargs=${bus_ids[1]} options:n_rxq="4"
[ -z  "$vf_id" ] && ovs-vsctl set port dpdkbond lacp=active
dpdk_ports=$(ovs-vsctl list int | grep name  | grep dpdk |awk '{print $3}')
for port in ${dpdk_ports[@]}
do
        ovs-vsctl set interface $port options:n_rxq=$dpdk_rxq
        echo "   $port.n_rxq:$dpdk_rxq"
done