#!/bin/bash
umount /mnt/sata
mkfs.ext4 /dev/block/$1 << EOF
y
EOF
echo mkfs=$?
#fileend
