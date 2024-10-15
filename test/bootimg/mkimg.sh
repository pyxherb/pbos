#!/bin/bash
set -e
imgpath="$PWD/build/boot.raw"

# Create original image.
dd if=/dev/zero bs=96M count=1 > $imgpath

# Grant privileges.
chmod 777 $imgpath

# Create partitions.
parted $imgpath << EOF
mklabel
msdos
mkpart
primary
ext2
1MiB
95MiB
EOF

# Loop device path.
loopDevName=$(losetup --show -f ${imgpath})

# Boot partition path.
bootFsName=$(losetup --show -f ${imgpath} -o 1048576 --sizelimit `expr 1024 \* 1024 \* 32`)

# Format partitions.
mke2fs "$bootFsName"

# Mount boot partition.
mount "$bootFsName" /media

# GRUB Installation.
grub-install \
--target=i386-pc \
--root-directory=/media \
--skip-fs-probe \
--no-rs-codes \
--modules="part_msdos fat ext2 elf iso9660 udf cat odc" \
"$loopDevName"

# Copy files.

cp ./test/bootimg/config/grub.cfg /media/boot/grub/
cat ./test/bootimg/config/49_pbos >> /media/boot/grub/grub.cfg

cp \
./build/pbkim \
./build/initcar \
/media/

# Unmount boot partition.
umount /media

# Delete loop devices.
losetup -d "$bootFsName"
losetup -d "$loopDevName"

echo "Generated image Successfully."
