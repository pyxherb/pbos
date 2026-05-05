#!/bin/bash
if [ -z "$LIMINE_PATH" ]; then
	echo "LIMINE_PATH does not present"
	exit 1
fi

if [ ! -d "$LIMINE_PATH" ]; then
	echo "LIMINE_PATH \"$LIMINE_PATH\" does not exist"
	exit 1
fi

imgpath="$PWD/build/boot.raw"

if [ ! -f "$imgpath" ]; then
	# Create original image.
	dd if=/dev/zero bs=256M count=1 > $imgpath

	# Grant privileges.
	chmod 777 $imgpath

	# Create partitions.
	parted $imgpath << EOF
mktable gpt
mkpart EFI fat32 1MiB 63MiB
mkpart PbOS fat32 64MiB 160MiB
toggle 1 esp
EOF
else
	imgInited=1
fi

# Loop device path.
loopDevName=$(losetup --show -f ${imgpath})

# Boot partition path.
bootFsName=$(losetup --show -f ${imgpath} -o 1048576 --sizelimit `expr 1024 \* 1024 \* 63`)

# System partition path.
systemFsName=$(losetup --show -f ${imgpath} -o `expr 1024 \* 1024 \* 64` --sizelimit `expr 1024 \* 1024 \* 160`)

if [ ! "$imgInited" ]; then
	# Format partitions.
	mkfs.vfat "$bootFsName"
	mkfs.vfat "$systemFsName"

	# Mount boot partition.
	mount "$bootFsName" /media
		# Limine Installation.
		mkdir "/media/EFI"
		mkdir "/media/EFI/BOOT"

		cp "$LIMINE_PATH/BOOTX64.EFI" "/media/EFI/BOOT"
		cp "$LIMINE_PATH/BOOTIA32.EFI" "/media/EFI/BOOT"
	# Unmount the partition.
	umount /media
fi

# Mount system partition.
mount "$systemFsName" /media
	# Copy files.
	cp "test/bootimg-x86_64/config/limine.conf" "/media"

mkdir /media/sys
# mkdir /media/sys/boot
# mkdir /media/sys/kernel

# cp ./build/initcar /media/sys/boot
cp ./build/kernel/pbkrnl /media/sys/pbkrnl
cp ./build/initcar /media/sys/initcar

# Unmount the partition.
umount /media

# Delete loop devices.
losetup -d "$systemFsName"
losetup -d "$bootFsName"
losetup -d "$loopDevName"
