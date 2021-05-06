#!/bin/bash
#set -x

if [ ! -L  /sys/class/net/service0/device ];then
echo "you have initlized, please use"
echo "/usr/sbin/dpdk-devbind -s"
echo "to check "
exit
fi

driver=$(readlink /sys/class/net/service0/device/driver|awk -F'/' '{print $NF}')

if [ "$driver" == "mlx5_core" ] ;then
#使用vf 做dpdk interface, 且开启lacp
uselinuxlacp=
lacpstr=active
vf_id=0
elif [ "$driver" == "igb" ];then
#使用vf 做dpdk interface, 且开启lacp
uselinuxlacp=
lacpstr=active
vf_id=0
elif [ "$driver" == "ixgbe" ];then
#使用vf 做dpdk interface, 且开启lacp
uselinuxlacp=
lacpstr=active
vf_id=
# x722 should use pf
elif [ "$driver" == "i40e" ];then
#使用pf 做dpdk interface, 且开启lacp
uselinuxlacp=
lacpstr=active
vf_id=
fi

if [ ! -z "$vf_id" ];then
#vf 做lacp协商
	echo 2 >  /sys/class/net/service0/device/sriov_numvfs
	echo 2 >  /sys/class/net/service1/device/sriov_numvfs
	bus_id_0=$(readlink /sys/class/net/service0/device/virtfn${vf_id}|awk -F'/' '{print $NF}')
	bus_id_1=$(readlink /sys/class/net/service1/device/virtfn${vf_id}|awk -F'/' '{print $NF}')
else
	bus_id_0=$(readlink /sys/class/net/service0/device|awk -F'/' '{print $NF}')
	bus_id_1=$(readlink /sys/class/net/service1/device|awk -F'/' '{print $NF}')
fi

bus_ids=($bus_id_0 $bus_id_1)

/usr/sbin/ip link set service0 nomaster
/usr/sbin/ip link set service1 nomaster
/usr/sbin/ifdown bond1
/usr/sbin/ip link delete  bond1

if [ ! -z "$uselinuxlacp" ] ; then
	cat > /etc/sysconfig/network-scripts/ifcfg-service0 <<EOF
DEVICE=service0
TYPE=Ethernet
ONBOOT=yes
BOOTPROTO=none
MASTER=bond1
SLAVE=yes
EOF
	cat > /etc/sysconfig/network-scripts/ifcfg-service1 <<EOF
DEVICE=service1
TYPE=Ethernet
ONBOOT=yes
BOOTPROTO=none
MASTER=bond1
SLAVE=yes
EOF

       cat > /etc/sysconfig/network-scripts/ifcfg-bond1 <<EOF
DEVICE=bond1
ONBOOT=yes
TYPE=Bond
BONDING_MASTER=yes
BONDING_OPTS="mode=4 miimon=100 xmit_hash_policy=layer3+4"
EOF
else
	cat > /etc/sysconfig/network-scripts/ifcfg-service0 <<EOF
DEVICE=service0
TYPE=Ethernet
ONBOOT=yes
BOOTPROTO=none
MASTER=bond1
SLAVE=yes
EOF
	cat > /etc/sysconfig/network-scripts/ifcfg-service1 <<EOF
DEVICE=service1
TYPE=Ethernet
ONBOOT=yes
BOOTPROTO=none
MASTER=bond1
SLAVE=yes
EOF

       cat > /etc/sysconfig/network-scripts/ifcfg-bond1 <<EOF
DEVICE=bond1
ONBOOT=yes
TYPE=Bond
BONDING_MASTER=yes
BONDING_OPTS="mode=2 miimon=100 xmit_hash_policy=layer3+4"
EOF
fi

/usr/sbin/ifup bond1
##modify qemu livirt ovs config

/usr/bin/sed -i 's/#user = "root"/user = "root"/g' /etc/libvirt/qemu.conf
/usr/bin/sed -i 's/#group = "root"/group = "root"/g' /etc/libvirt/qemu.conf
/usr/bin/sed -i '/^seccomp_sandbox=0/d' /etc/libvirt/qemu.conf
/usr/bin/sed -i '$a\seccomp_sandbox=0' /etc/libvirt/qemu.conf
##/usr/bin/sed -i '$a\namespaces=[]' /etc/libvirt/qemu.conf
/usr/bin/sed -i 's|OVS_USER_ID="openvswitch:hugetlbfs"|OVS_USER_ID="root:root"|g' /etc/sysconfig/openvswitch
/usr/bin/sed -i 's|OVS_USER_ID=openvswitch:hugetlbfs|OVS_USER_ID="root:root"|g' /run/openvswitch.useropts

/usr/bin/sed -i '/^ExecStartPre=-\/usr\/bin\/initvf\.sh/d'    /usr/lib/systemd/system/ovs-vswitchd.service
/usr/bin/sed -i '/^ExecStartPost=-\/usr\/bin\/initdpdk\.sh/d'    /usr/lib/systemd/system/ovs-vswitchd.service
systemctl restart ovsdb-server

/usr/bin/ovs-appctl -t ovs-vswitchd exit


echo 0 > /sys/class/net/service0/device/sriov_numvfs
echo 0 > /sys/class/net/service1/device/sriov_numvfs
echo 2 > /sys/class/net/service0/device/sriov_numvfs
echo 2 > /sys/class/net/service1/device/sriov_numvfs
sleep 5
echo "enable 2 vfs "

systemctl daemon-reload
systemctl start ovs-vswitchd

ovs-vsctl --may-exist add-br br-bond1 -- set Bridge br-bond1 datapath_type=netdev
echo "setting add br-bond1"
ovs-vsctl --may-exist add-br br-int -- set Bridge br-int datapath_type=netdev
echo "setting add br-int"
ovs-vsctl set Bridge br-bond1 other_config:mac-table-size="16384"
echo "change mac table size to 16384"

cmdline=$(cat /etc/default/grub | grep isolcpus)
isolcpus=$(echo ${cmdline#*'isolcpus='} | awk '{print $1}' | sed 's/,/ /g'|tr -d '"')
cpu_mask=0
for id in ${isolcpus[@]};do cpu_mask=$(($cpu_mask | 1 << $id));done
cpu_mask_hex=$(printf "0x%x" $cpu_mask)

ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=$cpu_mask_hex
echo "setting   pmd-cpu-mask:$cpu_mask_hex ${isolcpus[@]}"

#Documentation/intro/install/dpdk.rst
ovs-vsctl set Open_vSwitch . other_config:tx-flush-interval=50
echo "setting   tx-flush-interval=50"

ovs-vsctl set Open_vSwitch . other_config:mrg_rxbuf=off
echo "setting   mrg_rxbuf=off, we don't used jumbo frame"

#Documentation/topics/userspace-tso.rst
ovs-vsctl set Open_vSwitch . other_config:userspace-tso-enable=true
echo "setting   userspace-tso-enable=true,  ovs/tap qemu guest got offload feature -- csum tso4 tso6"

ovs-vsctl set Open_vSwitch . other_config:dpdk-init=true
echo "setting dpdk-init:true"

ovs-vsctl --may-exist add-br br-bond1 -- \
                        set Bridge br-bond1 datapath_type=netdev -- \
                        br-set-external-id br-bond1 bridge-id br-bond1  -- \
                        set bridge br-bond1 fail-mode=standalone
ovs-vsctl --may-exist add-bond br-bond1 dpdkbond dpdk0 dpdk1 bond_mode=balance-tcp \
			other_config:lacp-fallback-ab=true \
			other_config:lacp-time=fast \
			other_config:lb-output-action=true -- \
                        set Interface dpdk0 type=dpdk options:dpdk-devargs=${bus_ids[0]} options:n_rxq_desc=4096 -- \
                        set Interface dpdk1 type=dpdk options:dpdk-devargs=${bus_ids[1]} options:n_rxq_desc=4096

ovs-vsctl set port dpdkbond lacp=$lacpstr

dpdk_rxq=2
ovs-vsctl set interface dpdk0 options:n_rxq=$dpdk_rxq
ovs-vsctl set interface dpdk1 options:n_rxq=$dpdk_rxq
echo "setting   n_rxq:$dpdk_rxq"

declare vf=()
declare mac=()

#只有vf 组dpdk 时需要设置mac ?或者也不需要
if [ ! -z "$vf_id" ] ; then
	for k in 0 1;do
		for i in /sys/class/net/service${k}/device/virtfn*/net/*;do
			vf_device_name=$i
			busstr=`readlink  $vf_device_name/device`
			echo "$busstr"|grep "${bus_ids[$k]}" -q
			if [ $? == 0 ];then
				mac[$k]=`cat $i/address`
				vf[$k]=$vf_device_name
			fi
		done
	done

	echo "service0 vf0 is ${vf[0]}   mac: ${mac[0]}"
	echo "service1 vf0 is ${vf[1]}   mac: ${mac[1]}"
fi

#told to try
if [ ! -z "$vf_id" ] && [ "$driver" == "ixgbe" ]; then
ovs-vsctl set bridge br-bond1 other_config:hwaddr=${mac[0]}
fi

cat > /usr/bin/initvf.sh <<EOF
#!/bin/bash
#========================================================
#   DESCRIPTION: init vf's mac for ovsdpdk
#  ORGANIZATION: iaas
#       CREATED: 2020/04/02
#        AUTHOR: blameto Xieyinghao
#========================================================
set -x
driver=$driver
vf_id=$vf_id
bus_ids=("$bus_id_0" "$bus_id_1")
mac=("${mac[0]}" "${mac[1]}")

modprobe iavf
modprobe ixgbevf

if [ "\$driver" == "mlx5_core" ] ;then
	echo "mlx5_core does not need install  vfio-pci  module"
elif [ "\$driver" == "i40e" ] || [ "\$driver" == "ixgbe" ];then
	if [ ! -z "\$vf_id" ] ; then
		/usr/sbin/modprobe vfio-pci
		echo "install  vfio-pci  module"
		for id in \${bus_ids[@]};
		do
			/usr/sbin/dpdk-devbind -b vfio-pci \${id}
			cat "/sys/bus/pci/devices/\${id}/uevent" | grep -q "vfio-pci"
			if [ \$? -eq 0 ];then
				echo "binding vfio-pci driver \$id:vfio-pci";
			else
				echo "\$id:failed-to-bind";
				exit 1
			fi
		done
	else
		/usr/sbin/modprobe uio
		/usr/sbin/insmod /igb_uio.ko
		echo "install  uio  module"
		for id in \${bus_ids[@]};
		do
			/usr/sbin/dpdk-devbind -b \$driver \${id}
			/usr/sbin/dpdk-devbind -b igb_uio \${id}
			#ixgbe 无法使能vf
			if [ "\$driver" == "i40e" ];then
				echo 1 > /sys/bus/pci/devices/\${id}/max_vfs
				echo 1 > /sys/bus/pci/devices/\${id}/sriov_numvfs
			fi
			cat "/sys/bus/pci/devices/\${id}/uevent" | grep -q "igb_uio"
			if [ \$? -eq 0 ];then
				echo "binding igb_uio driver \$id:igb_uio";
			else
				echo "\$id:failed-to-bind";
				exit 1
			fi
		done
	fi
elif [ "\$driver" == "igb" ];then
	echo "todo"
fi

if [ ! -z \$vf_id ] ; then

	for k in 0 1;do
	if [ "\$driver" == "ixgbe" ];then
		:;#ip link set dev service\$k vf \$vf_id mac \${mac[\k]}
	else
		:;#ip link set dev service\$k vf \$vf_id mac \${mac[\$k]}
	fi
	ip link set dev service\$k vf \$vf_id trust on
	ip link set dev service\$k vf \$vf_id state enable
	ip link set dev service\$k vf \$vf_id spoof  off
	ip link set dev service\$k vf \$vf_id vlan 0
	done

	relmac0=\`ip link show service0  |grep "vf 0"|awk '{print \$4}'|tr -d ","\`
	relmac1=\`ip link show service1  |grep "vf 0"|awk '{print \$4}'|tr -d ","\`

	if [ \${relmac0} == \${mac[0]} ] && [ \${relmac1} == \${mac[1]} ];then

	echo "dpdk vf init ok"

	else
	echo expecting  \${mac[0]} \${mac[1]} got \$relmac0 \$relmac1
	echo "dpdk vf mac init failed?? continue....."

	fi
fi

EOF

cat > /usr/bin/initdpdk.sh <<EOF
#!/bin/bash
#========================================================
#   DESCRIPTION: init configuration for ovsdpdk
#  ORGANIZATION: iaas
#       CREATED: 2020/04/02
#        AUTHOR: blameto Xieyinghao
#========================================================
set -x
driver=$driver
vf_id=$vf_id
bus_ids=("$bus_id_0" "$bus_id_1")
EOF

chmod a+x /usr/bin/initvf.sh
chmod a+x /usr/bin/initdpdk.sh

/usr/bin/sed -i '/^ExecStartPre=-\/usr\/bin\/initvf\.sh/d'    /usr/lib/systemd/system/ovs-vswitchd.service
/usr/bin/sed -i '/^ExecStartPost=-\/usr\/bin\/initdpdk\.sh/d'    /usr/lib/systemd/system/ovs-vswitchd.service
/usr/bin/sed -i '/^ExecStart=/i \ExecStartPre=-/usr/bin/initvf.sh'    /usr/lib/systemd/system/ovs-vswitchd.service
/usr/bin/sed -i '/^ExecStop=/i \ExecStartPost=-/usr/bin/initdpdk.sh'    /usr/lib/systemd/system/ovs-vswitchd.service

systemctl daemon-reload
systemctl restart openvswitch
systemctl restart neutron-openvswitch-agent
