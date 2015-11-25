#!/bin/bash
umount /mnt/sata
mkfs.exfat /dev/block/$1 << EOF
EOF
echo mkfs=$?
#fileend
