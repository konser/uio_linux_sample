#!/usr/bin/env python
# -*- coding: utf-8 -*-

import libvirt
import commands
import sys
from lxml import etree
import six

conn = libvirt.open(None)
if conn is None:
	print('Failed to open connection to the hypervisor')
	sys.exit(1)
objlist=conn.listAllDomains(0)

conn.close()

hostname="xgsito02010243080022.cnsuning.com"
hostip="10.243.80.22"
vmuuid="925e228d-b73f-4e01-b1e5-9fc24b891313"
testdom=None

for i in objlist:
	uuid=i.UUIDString()
	if uuid == vmuuid:
		testdom = i
		print 'migrating %s' % uuid
		break
def remote_cmd(hostname, cmd):
	_cmd="ssh -p 22 root@%s '%s'" % (hostname, cmd)
	print _cmd
	ret=commands.getoutput(_cmd)
	return ret

def file_exist(file):
	ret=remote_cmd(hostname, 'ls %s' % file)
	print ret,file
	if file in ret and ('No such file' not in ret):
		print '%s exsit ,skip' % file
		return True
	else:
		return False
dirpath="/os_instance/%s/" % testdom.UUIDString()

remote_cmd(hostname, 'mkdir %s' % dirpath)

#检查系统盘
diskfile='%sdisk' % dirpath

if not file_exist(diskfile):
	cmd='scp %s %s:%s' % (diskfile, hostname, dirpath)
	print cmd
	commands.getoutput(cmd)
#检查光盘
configfile='%sdisk.config' % dirpath

if not file_exist(configfile):
	cmd='scp %s %s:%s' % (configfile, hostname, dirpath)
	print cmd
	commands.getoutput(cmd)
#获取模板
basefile=""
cmd='qemu-img info  %sdisk' % (dirpath)
print cmd
ret=commands.getoutput(cmd)
for l in ret.split("\n"):
	if 'backing file:' in l:
		basefile=l.split(':')[1].strip()

if basefile and not file_exist(basefile):
	os.exit()
	cmd='scp %s  %s:%s' % (basefile, hostname, '/os_instance/_base/')
	print cmd
	commands.getoutput(cmd)
'''
#检查qbr
cmd="virsh domiflist %s|grep bridge|awk '{print $3}'" % vmuuid
qbrdevice=commands.getoutput(cmd)

#远端创建qbr
remote_cmd(hostname, 'brctl addbr %s' % qbrdevice)
'''

migratable_flag = getattr(libvirt, 'VIR_DOMAIN_XML_MIGRATABLE',
                                          None)
new_xml_str=i.XMLDesc(migratable_flag)

print new_xml_str

def _new_node(node_name, **kwargs):
	return etree.Element(node_name, **kwargs)

def _text_node(node_name, value, **kwargs):
	child = _new_node(node_name, **kwargs)
	child.text = six.text_type(value)
	return child

root = etree.fromstring(new_xml_str)

#dst guest cputune 加vcpupin  start
cputune=root.findall("cputune")[0]

for i in range(0,8):
	vcpupin=etree.Element("vcpupin")
	vcpupin.set("vcpu",str(i))
	vcpupin.set("cpuset","8-15,24-31")
	cputune.append(vcpupin)
	#cputune.append(_text_node("vcpupin",vcpupin.text))

print etree.tostring(cputune, pretty_print=True)
#dst guest cputune 加vcpupin  end

#dst guest 加memoryBacking  end
memoryBacking=etree.Element("memoryBacking")
hugepages=etree.Element("hugepages")
page=etree.Element("page")
page.set("size","2048")
page.set("unit","KiB")
page.set("nodeset","0")

hugepages.append(page)
memoryBacking.append(hugepages)

print etree.tostring(memoryBacking, pretty_print=True)
root.append(memoryBacking)

#dst guest 加memoryBacking  end

#dst guest 加numatune start
numatune=etree.Element("numatune")
memory=etree.Element("memory")
memory.set("mode","strict")
memory.set("nodeset","1")

memnode=etree.Element("memnode")
memnode.set("cellid","0")
memnode.set("nodeset","1")
memnode.set("mode","strict")

numatune.append(memory)
numatune.append(memnode)

print etree.tostring(numatune, pretty_print=True)
root.append(numatune)

#dst guest 加numatune end

#dst guest 给cpu节增加numa start
cpu=root.findall("cpu")[0]
numa=etree.Element("numa")
cell=etree.Element("cell")
cell.set("id","0")
cell.set("cpus","0-7")
cell.set("memory","8388608")
cell.set("unit","KiB")
cell.set("memAccess","shared")
numa.append(cell)
print etree.tostring(numa, pretty_print=True)
cpu.append(numa)

#dst guest 给cpu节增加numa end


devices=root.findall("./devices")[0]
interface=devices.findall("interface")[0]

originmac=interface.findall("mac")[0].get("address")

mac=etree.Element("mac")
mac.set("address",originmac)
devname=interface.findall("target")[0].get("dev").replace("tap","vhu")

#把dst guest 接口接入ovs  start

remote_cmd(hostname,"ovs-vsctl add-port br-int %s" % devname)
remote_cmd(hostname,'ovs-vsctl set Interface %s type=dpdkvhostuserclient options={vhost-server-path="/var/run/openvswitch/%s"}' % (devname,devname))
remote_cmd(hostname,'ovs-vsctl set Port %s tag=1' % devname)
'''
remote_cmd(hostname,'ovs-vsctl set Port %s other_config:tag=1' % devname)
remote_cmd(hostname,'ovs-vsctl set Port %s other_config:network_type=vlan' % devname)
remote_cmd(hostname,'ovs-vsctl set Port %s other_config:physical_network=physnet1' % devname)
remote_cmd(hostname,'ovs-vsctl set Port %s other_config:segmentation_id=101' % devname)
remote_cmd(hostname,'ovs-vsctl set Port %s other_config:net_uuid=xx' % devname)
'''
remote_cmd(hostname,'ovs-vsctl set Interface %s external_ids:attached-mac=%s' % (devname,originmac))

#把dst guest 接口接入ovs  end

print etree.tostring(interface, pretty_print=True)
interface.clear()
print etree.tostring(interface, pretty_print=True)

#dst guest 替换为dpdk 类型接口 start
interface.set("type","vhostuser")

target=etree.Element("target")
target.set("dev",devname)

source=etree.Element("source")
source.set("type","unix")
source.set("path","/var/run/openvswitch/"+devname)
source.set("mode","server")

model=etree.Element("model")
model.set("type","virtio")

driver=etree.Element("driver")
driver.set("name","vhost")
driver.set("rx_queue_size","8192")

#老架构开启了csum tso4 tso6, dpdk vhost-user 默认关闭offload 需要显式开启
#同时为了跨架构兼容，需定制qemu, 在virtio_load 流程, 复位 virio-net guest_features 的两个标志
# VIRTIO_NET_F_HOST_ECN VIRTIO_NET_F_HOST_UFO
#ovs-vsctl set Open_vSwitch . other_config:userspace-tso-enable=true

guest=etree.Element("guest")
guest.set("csum","on")
guest.set("tso4","on")
guest.set("tso6","on")

host=etree.Element("host")
host.set("csum","on")
host.set("tso4","on")
host.set("tso6","on")
host.set("ecn","off")
host.set("ufo","off")
driver.append(guest)
#driver.append(host)

alias=etree.Element("alias")
alias.set("name","net0")

interface.append(mac)
interface.append(source)
interface.append(target)
interface.append(model)
interface.append(driver)
interface.append(alias)

#dst guest 替换为dpdk 类型接口 end


#dst guest 替换graphics 监听地址 start
graphics=root.findall("./devices/graphics")[0]
graphics.set("port","-1")
graphics.set("listen",hostip)

l=graphics.findall("listen")[0]
l.set("address",hostip)

new_xml_str=etree.tostring(root, pretty_print=True)
print new_xml_str

#dst guest 替换graphics 监听地址 end

migrate_uri="qemu+tcp://%s/system" % hostname
device_names=['vda',]

params= {
# 'migrate_uri': migrate_uri,
 'destination_xml': new_xml_str,
 'migrate_disks': device_names,
 'persistent_xml':new_xml_str,
# 'statistic-pullout-for-migrate':1,
}

try:
	#testdom.migrateToURI2(migrate_uri, None, new_xml_str, 0x289b, None, 0)  #OK
	ret=testdom.migrateToURI3(migrate_uri, params, flags=0x289b)		#OK
        #ret=testdom.migrateToURI2_Ex(migrate_uri, None, new_xml_str, 0x289b, None, 0)  #OK
	#ret=testdom.migrateToURI3_Ex(migrate_uri, params,flags=0x289b)
	print ret
except Exception as e:
	print e
#testdom.migrateToURI3_Ex(dconnuri,params,0,0,0)
