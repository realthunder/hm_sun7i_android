#!/bin/bash

ls /sys/devices/platform/sw_ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/

if [ $? = 1 ]
then	
echo "no sata"
exit 1
fi

fdisk /dev/block/$1 << EOF

n
p
$2

+$3M

w
EOF
echo fdisk=$?

#fileend
