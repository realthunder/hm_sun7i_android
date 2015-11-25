#!/bin/bash
mkdir /mnt/sd

mkfs.ext4 /dev/block/$1$2 <<EOF
y
EOF

if [ $? = 1 ]
then
echo no sata
exit 1 
fi

mount -t ext4 /dev/block/$1$2 /mnt/sd

if [ $? = 1 ]
then
exit 1
fi

cp -a /system/* /mnt/sd

sync

umount /mnt/sd

sync

rm -rf /mnt/sd
echo all finish cp=$?

#fileend
