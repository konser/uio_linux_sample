set -x

#清理ovsdpdk-init.sh 接口配置改动
/usr/sbin/ifdown bond1
/usr/sbin/ifdown service0
/usr/sbin/ifdown service1
/usr/sbin/ip link delete  bond1
rm -f /etc/sysconfig/network-scripts/ifcfg-bond1

sed -i '/^MASTER=bond1/d' /etc/sysconfig/network-scripts/ifcfg-service0
sed -i '/^SLAVE=yes/d'  /etc/sysconfig/network-scripts/ifcfg-service0
sed -i '/^MASTER=bond1/d' /etc/sysconfig/network-scripts/ifcfg-service1
sed -i '/^SLAVE=yes/d' /etc/sysconfig/network-scripts/ifcfg-service1

/usr/sbin/ifup service0
/usr/sbin/ifup service1

#清理db
mv /etc/openvswitch/conf.db /etc/openvswitch/conf.db_$(date +%Y%m%d%H%M%S)
systemctl stop openvswitch
systemctl restart ovsdb-server

#清理ovsdpdk-init.sh 服务改动
/usr/bin/sed -i '/^ExecStartPre=-\/usr\/bin\/initvf\.sh/d'    /usr/lib/systemd/system/ovs-vswitchd.service
/usr/bin/sed -i '/^ExecStartPost=-\/usr\/bin\/initdpdk\.sh/d'    /usr/lib/systemd/system/ovs-vswitchd.service
systemctl daemon-reload
systemctl start openvswitch

#清理残余设备
/usr/sbin/ip link delete br-bond1
/usr/sbin/ip link delete br-int

#ovs 模式初始化
ovs-vsctl --may-exist add-br br-bond1
ovs-vsctl --may-exist add-bond br-bond1 bond1 service0 service1

ovs-vsctl set Port bond1 bond_mode=balance-tcp lacp=active other_config:lacp-time=fast

ovs-vsctl set bridge br-bond1 other_config:mac-table-size="16384"


#修改插件配置

/usr/bin/sed -i  '/^vhostuser_socket_dir=\/var\/run\/openvswitch/d'  /etc/neutron/plugins/ml2/openvswitch_agent.ini
/usr/bin/sed -i  '/^datapath_type= netdev/d'  /etc/neutron/plugins/ml2/openvswitch_agent.ini

systemctl restart neutron-openvswitch-agent

systemctl restart ovs-vswitchd.service
systemctl restart openstack-nova-compute
systemctl restart neutron-openvswitch-agent
systemctl restart openstack-cinder-volume
