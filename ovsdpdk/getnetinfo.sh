#!/bin/bash

cpu=$(lscpu|grep "Model name"|cut -d':' -f2|tr -d " ")
out="{ \"cpu\":\"$cpu\""
tmp=`mktemp`
yum install lshw -y &>/dev/null
/usr/sbin/lshw -class network -businfo > $tmp 
while read _line;do
line=`echo $_line|grep "^pci"|egrep "mgmt|service"`
if [ ! -z "$line" ];then
	name=$(echo $line |awk '{print $2}')
	dev=$(echo $line |awk '{for(i=4;i<=NF;++i) print $i}')
	out="$out,\"$name\":\"$dev\""
fi
done < $tmp
out="$out }"
echo $out
rm -f $tmp
